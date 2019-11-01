#pragma once

#include "Arduino.h"

void TaskSensor(void *pvParameters);

class TMP3X
{
    public:
        TMP3X();

        int readTemp();
        static double getTemp();
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