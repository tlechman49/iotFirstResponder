#include "sensor_task.hpp"
#include "Adafruit_CCS811.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"

//set statics at compile time
double TMP3X::_temp = 0;
uint16_t CCS_CO2::_co2 = 0;

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
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
}

//reads temperature from pin 36 only.. can be changed by changing the channel .. but this should be fine
int TMP3X::readTemp()
{
    int adcval = adc1_get_raw(ADC1_CHANNEL_0);

    //check if it gave a bad value
    if (adcval == -1)
    {
        return 1;
    }

    //convert to volts then to celcius
    double v = (adcval / 4095.0) * 1.1; //in volts
    _temp = (v - 0.5) * 100;      // in celcius
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
    while (!ccs.available() && availTimeout < 30) // 3 seconds
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
            return co2_level;
        }
        else
        {
            return -1;
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
