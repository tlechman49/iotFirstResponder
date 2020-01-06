#include "sensor_task.hpp"
#include "Adafruit_CCS811.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "wifi_task.hpp"
#include "main.hpp"

// short and long delays for sensor reads in ms
#define USE_DYNAMIC_TIMING 1
#define SHORT_DELAY 5000 
#define LONG_DELAY  10000

//set statics at compile time
uint16_t Flame::_flame = 0;
uint16_t TMP3X::_temp = 0;
uint16_t CCS_CO2::_co2 = 0;

adc1_channel_t TMP3X::channel = ADC1_CHANNEL_0;     //GPIO36
adc_atten_t TMP3X::atten = ADC_ATTEN_DB_0;
adc_unit_t TMP3X::unit = ADC_UNIT_1;
esp_adc_cal_characteristics_t * TMP3X::adc_chars = NULL;

adc1_channel_t Flame::channel = ADC1_CHANNEL_3;     //GPIO39
adc_atten_t Flame::atten = ADC_ATTEN_DB_11;
adc_unit_t Flame::unit = ADC_UNIT_1;
esp_adc_cal_characteristics_t * Flame::adc_chars = NULL;
uint32_t Flame::flameThresh = 1250; //arbitrary threshold

Adafruit_CCS811 ccs;

// sensor task function
void TaskSensor(void *pvParameters) 
{
    // prevent unused message
    (void)pvParameters;

    // initialization values
    uint32_t ulNotifiedValue;
    uint8_t useFakeData = 0;

    // loop variables
    char message[32] = "0,0,0";
    uint16_t lastSentTemp = -100; // initializing to unlikely values
    uint16_t lastSentCo2 = -1; // initializing to unlikely values
    uint16_t lastSentFlame = -1;

    //wait until the system is informed to start
    xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                     ULONG_MAX, /* Reset the notification value to 0 on exit. */
                     &ulNotifiedValue, /* Notified value pass out in
                                          ulNotifiedValue. */
                     portMAX_DELAY );  /* Block indefinitely. */

    // set some parameters used mainly for debugging
    if( ( ulNotifiedValue & 0x01 ) != 0 )
    {
        /* Bit 0 was set - process whichever event is represented by bit 0. */
        useFakeData = 1;
    }

    CCS_CO2 co2;
    TMP3X tmp;
    Flame fl;

    // initialize sensors if not using fake data
    if (useFakeData == 0)
    {
        co2.begin();
    }

    //task loop
    for (;;)
    {
        //generate fake data
        if (useFakeData)
        {
            co2.setCo2(millis() % 800);
            tmp.setTemp(millis() % 40);
            fl.setFlame(millis() % 1);
        }
        // record sensor data
        else
        {
            co2.readCo2();
            tmp.readTemp();
            fl.readFlame();
        }
        
        // check if the sensor values have changed
        uint8_t isValChanged = (isValueChanged(lastSentTemp/10, tmp.getTemp()/10) |
                                isValueChanged(lastSentCo2/10, co2.getCo2()/10) |
                                isValueChanged(lastSentFlame, fl.getFlame()));
        if ((USE_DYNAMIC_TIMING == 0) || isValChanged)
        {
            //prepare message
            //format is "co2,temp,flame"
            sprintf(message, "%u,%u.%u,%u", co2.getCo2(), tmp.getTemp()/10, tmp.getTemp()%10, fl.getFlame()); 
            wifi_task::setMessage(message);

            //tell wifi task to transmit
            notifyWiFiAndWait((FROM_SENSOR | TRANSMIT_TCP), NULL, portMAX_DELAY);

            vTaskDelay(SHORT_DELAY);
        }
        else
        {
            vTaskDelay(LONG_DELAY);
        }
    }
}

// checks if the values are equal
// to be readjusted later to some degree (10% or something)
int isValueChanged(uint16_t last, uint16_t current)
{
    return !(last == current);
}

//initializes the temperature sensor
TMP3X::TMP3X()
{
    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, 1100, adc_chars);
}

//reads temperature from pin 36 only.. can be changed by changing the channel .. 
// but this should be fine
// 0 == SUCCESS
int TMP3X::readTemp()
{
    // get raw adc value
    uint32_t adc_reading = 0;
    adc_reading = adc1_get_raw((adc1_channel_t)channel);

    //calibarate vs adc characteristics
    uint32_t V =  esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    //convert to (celcius *10)
    _temp = (uint16_t)(V-500);      // in (celcius * 10)
    return 0;
}

//gets the last read value from the temperature sensor
// (celcius * 10)
uint16_t TMP3X::getTemp()
{
    return _temp; 
}

// set temperature value for debugging purposes
// (celcius * 10)
void TMP3X::setTemp(uint16_t temp)
{
    _temp = temp;
}

// initialize c02 sensor
// 0 == SUCCESS
int CCS_CO2::begin()
{
    int availTimeout = 0;
    if (!ccs.begin())
    {
        return 1;
    }
    while (!ccs.available() && availTimeout < 100) // 3 seconds
    {
        vTaskDelay(100);
        availTimeout++;
    }
    return 0;
}

// reads the co2 sensor and stores to static
// 0 == SUCCESS
int CCS_CO2::readCo2()
{
    if (ccs.available())
    {
        if (!ccs.readData())
        {
            uint16_t co2_level = ccs.geteCO2();
            _co2 = co2_level;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return -1;
    }
}

// used to asynchronously get the last read co2 value
uint16_t CCS_CO2::getCo2()
{
    return _co2;
}

// manually sets the co2 value for debugging purposes
void CCS_CO2::setCo2(uint16_t co2)
{
    _co2 = co2;
}


Flame::Flame()
{
    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, 1100, adc_chars);
}

//reads flame from pin 39 only.. can be changed by changing the channel .. but this should be fine
int Flame::readFlame()
{
    uint32_t adc_reading = 0;
    adc_reading = adc1_get_raw((adc1_channel_t)channel);
    uint32_t V =  esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    //convert
    _flame = (uint16_t)(V < flameThresh);      
    return 0;
}

//gets the last read value from the ~flame~ sensor 
uint16_t Flame::getFlame()
{
    return _flame; 
}

void Flame::setFlame(uint16_t flame)
{
    _flame = flame;
}