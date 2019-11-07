#pragma once

#include "Arduino.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

void TaskSensor(void *pvParameters);

class TMP3X
{
    public:
        TMP3X();

        int readTemp();
        static double getTemp();
        static adc1_channel_t channel ;    //GPIO34 if ADC1, GPIO14 if ADC2
        static adc_atten_t atten ;
        static adc_unit_t unit;
        static esp_adc_cal_characteristics_t *adc_chars;

    private:
        static double _temp;
};

class CCS_CO2
{
    public:
        int begin();

        int readCo2();
        static uint16_t getCo2();
    private:
        static uint16_t _co2;
};