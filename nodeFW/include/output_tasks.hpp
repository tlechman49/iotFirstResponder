#pragma once

#include "Arduino.h"
#include "FastLED.h"

void createOutputTasks();

void TaskOnboardLed(void *pvParameters);
void TaskLedStrip(void *pvParameters);

FASTLED_USING_NAMESPACE

#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 64
#define BRIGHTNESS 64
#define FRAMES_PER_SECOND 60
#define LED_PIN 13

class LED
{
    public:
        LED(int pin);
        void wipe(int dir);
        void allOff();
        void show();
    private:
        int _dir;
        CRGB _leds[NUM_LEDS] = {CRGB::Black};
};