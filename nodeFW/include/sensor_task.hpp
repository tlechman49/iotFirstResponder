#pragma once

#include "Arduino.h"

void TaskSensor(void *pvParameters);

class TMP3X
{
    public:
        TMP3X();
        ~TMP3X();

        static int readTemp();
        static double getTemp();
    private:
        static double _temp;
};

class CCS_CO2
{
    public:
        CCS_CO2();
        ~CCS_CO2();

        int begin();

        static int readCo2();
        static uint16_t getCo2();
    private:
        static uint16_t _co2;
};