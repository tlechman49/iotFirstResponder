#include "output_tasks.hpp"
#include "Arduino.h"
#include "main.hpp"
#include "FastLED.h"

#define LED_PIN 13
#define ON_TIME_MS 200

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// create the output tasks
void createOutputTasks()
{
    // Create the output tasks here
    xTaskCreatePinnedToCore(
        TaskOnboardLed, "TaskOnboardLed" // A name just for humans
        ,
        4096 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &taskHandleOnboardLed, ARDUINO_RUNNING_CORE);

    // Create the output tasks here
    xTaskCreatePinnedToCore(
        TaskLedStrip, "TaskLedStrip" // A name just for humans
        ,
        4096 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &taskHandleLedStrip, ARDUINO_RUNNING_CORE);
}

// blinks onboard led based on the given period
// if period is less than 400ms, set it to 400ms
// if 0 stop the blinking
void TaskOnboardLed(void *pvParameters) 
{
    (void)pvParameters;
    uint32_t ulNotifiedValue = 0;
    TickType_t offTime = portMAX_DELAY;

    pinMode(LED_PIN, OUTPUT);

    for (;;) // A Task shall never return or exit.
    {
        // ulNotifiedValue is set to set the period of the onboard led blinks
        if (pdPASS == xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                                  ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                  &ulNotifiedValue, /* Notified value pass out in
                                                       ulNotifiedValue. */
                                 offTime )) // time spent blocking
        {
            // block the task indefinitely if the notification is 0
            if (ulNotifiedValue == 0)
            {
                offTime = portMAX_DELAY;
            }
            else 
            {
                // check if period is less than 400ms and set it to 400ms if it is
                if (ulNotifiedValue < 400) ulNotifiedValue = 400;
                offTime = pdMS_TO_TICKS(ulNotifiedValue - ON_TIME_MS);
            }
        }

        // blink the led
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(ON_TIME_MS);
        digitalWrite(LED_PIN, LOW);
    }
}

// wipes an animation either away from or toward the connection point of the led strip
// sets the leds to black and goes into block state if the animation is stopped
void TaskLedStrip(void *pvParameters) 
{
    (void)pvParameters;
    uint32_t ulNotifiedValue = 0;
    TickType_t offTime = portMAX_DELAY;
    int dir = 0;
    LED led(LED_PIN);

    for (;;) // A Task shall never return or exit.
    {
        // ulNotifiedValue is set to set the period of the onboard led blinks
        if (pdPASS == xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                                  ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                  &ulNotifiedValue, /* Notified value pass out in
                                                       ulNotifiedValue. */
                                 offTime )) // time spent blocking
        {
            // block the task indefinitely if the notification is 0
            if (ulNotifiedValue == 0)
            {
                led.allOff();
                offTime = portMAX_DELAY;
                continue;
            }
            else if (ulNotifiedValue == 1)
            {
                offTime = pdMS_TO_TICKS(1000 / FRAMES_PER_SECOND);
                dir = 0;
            }
            else if (ulNotifiedValue == 2)
            {
                offTime = pdMS_TO_TICKS(1000 / FRAMES_PER_SECOND);
                dir = 1;
            }
        }
        //update the animation 
        led.wipe(dir);
        led.show();
    }
}

LED::LED(const int pin)
{
    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE, 13, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    fill_solid(_leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}


// brightest led propogates across strip
// _dir == 0 -> forward
// _dir == 1 -> backward
void LED::wipe(int dir)
{
    _dir = dir;
    
    fadeToBlackBy( _leds, NUM_LEDS, 60);
    uint8_t u = (beat8(10, 0) % NUM_LEDS);
    if (_dir)
    {
        u = map( u, 0, NUM_LEDS-4, NUM_LEDS-4, 0 );
    }
    _leds[u] += CRGB::Green;
}

void LED::allOff()
{
    fill_solid(_leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void LED::show()
{
    FastLED.show();
}

