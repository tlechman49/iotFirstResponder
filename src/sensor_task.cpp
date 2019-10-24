#include "sensor_task.hpp"
#include "Adafruit_CCS811.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"

Adafruit_CCS811 ccs;

//gets temperature from pin 36 only.. can be changed by changing the channel .. but this should be fine
double get_temp()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
    int adcval = adc1_get_raw(ADC1_CHANNEL_0);
    double v = (adcval / 4095.0) * 1.1; //in volts
    double temp = (v - 0.5) * 100;      // in celcius
    return temp;
}

// gets the co2 level
//things to do: add include literal argument -i for initalize -g get shit... cool!
uint16_t get_co2()
{
    if (init_co2())
    {
        return -1;
    }
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

int init_co2()
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
