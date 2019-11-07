#include "sensor_task.hpp"
#include "Adafruit_CCS811.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

//set statics at compile time
double TMP3X::_temp = 0;
uint16_t CCS_CO2::_co2 = 0;

adc1_channel_t TMP3X::channel = ADC1_CHANNEL_0;     //GPIO34 if ADC1, GPIO14 if ADC2
adc_atten_t TMP3X::atten = ADC_ATTEN_DB_0;
adc_unit_t TMP3X::unit = ADC_UNIT_1;
esp_adc_cal_characteristics_t * TMP3X::adc_chars = NULL;

Adafruit_CCS811 ccs;


// sensor task function
void TaskSensor(void *pvParameters) 
{
    (void)pvParameters;

    //task loop
    for (;;)
    {
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

    //convert to volts then to celcius
    _temp = (V-500)/10.0;      // in celcius
    return 0;
}

//gets the last read value from the temperature sensor
double TMP3X::getTemp()
{
    return _temp; 
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
