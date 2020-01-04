#include "sensor_task.hpp"
#include "Adafruit_CCS811.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "wifi_task.hpp"
#include "main.hpp"

//set statics at compile time
double TMP3X::_temp = 0;
int Flame::_flame = 0;
uint16_t CCS_CO2::_co2 = 0;

adc1_channel_t TMP3X::channel = ADC1_CHANNEL_0;     //GPIO36
adc_atten_t TMP3X::atten = ADC_ATTEN_DB_0;
adc_unit_t TMP3X::unit = ADC_UNIT_1;
esp_adc_cal_characteristics_t * TMP3X::adc_chars = NULL;

adc1_channel_t Flame::channel = ADC1_CHANNEL_3;     //GPIO39
adc_atten_t Flame::atten = ADC_ATTEN_DB_11;
adc_unit_t Flame::unit = ADC_UNIT_1;
esp_adc_cal_characteristics_t * Flame::adc_chars = NULL;
uint32_t Flame::flameThresh = 1250; //Conversion of 512 to 1250mV

Adafruit_CCS811 ccs;

// sensor task function
void TaskSensor(void *pvParameters) 
{
    (void)pvParameters;
    uint32_t ulNotifiedValue;
    uint8_t useFakeData = 0;
    char message[32] = "0,0,0";

    //wait until the system is informed to start
    xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                     ULONG_MAX, /* Reset the notification value to 0 on exit. */
                     &ulNotifiedValue, /* Notified value pass out in
                                          ulNotifiedValue. */
                     portMAX_DELAY );  /* Block indefinitely. */

    /* Process any events that have been latched in the notified value. */
    if( ( ulNotifiedValue & 0x01 ) != 0 )
    {
        /* Bit 0 was set - process whichever event is represented by bit 0. */
        useFakeData = 1;
    }

    CCS_CO2 co2;
    TMP3X tmp;

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
            CCS_CO2::setCo2(millis() % 800);
            TMP3X::setTemp(millis() % 40);
        }
        // record sensor data
        else
        {
            co2.readCo2();
            tmp.readTemp();
        }
        
        //prepare message
        sprintf(message, "%u,%f,%u", CCS_CO2::getCo2(), TMP3X::getTemp(), 0); //format is "co2,temp,flame"
        wifi_task::setMessage(message);

        //tell wifi task to transmit
        notifyWiFiAndWait((FROM_SENSOR | TRANSMIT_TCP), NULL, portMAX_DELAY);

        vTaskDelay(10000);
    }
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
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, 1100, adc_chars);
    //print_char_val_type(val_type);

}

//reads temperature from pin 36 only.. can be changed by changing the channel .. but this should be fine
int TMP3X::readTemp()
{
    uint32_t adc_reading = 0;
    adc_reading = adc1_get_raw((adc1_channel_t)channel);
    double V =  esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    //convert to celcius
    _temp = (V-500)/10.0;      // in celcius
    return 0;
}

//gets the last read value from the temperature sensor
double TMP3X::getTemp()
{
    return _temp; 
}

void TMP3X::setTemp(double temp)
{
    _temp = temp;
}

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

uint16_t CCS_CO2::getCo2()
{
    return _co2;
}

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
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, 1100, adc_chars);
    //print_char_val_type(val_type);

}

//reads flame from pin 39 only.. can be changed by changing the channel .. but this should be fine
int Flame::readFlame()
{
    uint32_t adc_reading = 0;
    adc_reading = adc1_get_raw((adc1_channel_t)channel);
    uint32_t V =  esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    //convert to binary
    _flame = (int)(V < flameThresh);      
    return 0;
}

//gets the last read value from the ~flame~ sensor 
int Flame::getFlame()
{
    return _flame; 
}

void Flame::setFlame(int flame)
{
    _flame = flame;
}