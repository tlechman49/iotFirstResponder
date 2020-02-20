#pragma once

#include "Arduino.h"
#include "FastLED.h"

void TaskOutputManager(void *pvParameters);

//stores commands to be sent
struct command_s
{
    uint32_t    type;
    uint32_t    pin;
    uint32_t    msg;
};

//stores tasks that have been created
struct outTask_s
{
    uint32_t        type{(uint32_t) -1 };
    uint32_t        pin{ 0 };
    TaskHandle_t    handle{ NULL };
};

int createNewTask(command_s cmd, outTask_s * taskArray);

void TaskOnboardLed(void *pvParameters);
void TaskLedStrip(void *pvParameters);
void TaskServo(void *pvParameters);
void TaskWater(void *pvParameters);

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

class SERVO
{
    public:
        SERVO(int pin);
        void setPosition(int position);
        int pin;
        int channel;
    private:
        static int _channelCount;
};