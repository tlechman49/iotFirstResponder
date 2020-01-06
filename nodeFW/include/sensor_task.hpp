#pragma once

#include "Arduino.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

void TaskSensor(void *pvParameters);

int isValueChanged(uint16_t last, uint16_t current);
class TMP3X
{
    public:
        TMP3X();

        static adc1_channel_t channel;
        static adc_atten_t atten;
        static adc_unit_t unit;
        static esp_adc_cal_characteristics_t *adc_chars;

        int readTemp();
        static uint16_t getTemp();
        static void setTemp(uint16_t temp);

    private:
        static uint16_t _temp;
};

class CCS_CO2
{
    public:
        int begin();

        int readCo2();
        static uint16_t getCo2();
        static void setCo2(uint16_t co2);
    private:
        static uint16_t _co2;
};

class Flame
{
    public:
        Flame();

        static adc1_channel_t channel;
        static adc_atten_t atten;
        static adc_unit_t unit;
        static esp_adc_cal_characteristics_t *adc_chars;

        static uint32_t flameThresh;

        int readFlame();
        static uint16_t getFlame();
        static void setFlame(uint16_t flame);

    private:
        static uint16_t _flame;
};