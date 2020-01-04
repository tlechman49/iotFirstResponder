#pragma once

#include "Arduino.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

void TaskSensor(void *pvParameters);

class TMP3X
{
    public:
        TMP3X();

        static adc1_channel_t channel;
        static adc_atten_t atten;
        static adc_unit_t unit;
        static esp_adc_cal_characteristics_t *adc_chars;

        int readTemp();
        static double getTemp();
        static void setTemp(double temp);

    private:
        static double _temp;
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
        static int getFlame();
        static void setFlame(int flame);

    private:
        static int _flame;
};