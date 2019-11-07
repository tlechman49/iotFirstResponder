#include "output_tasks.hpp"
#include "Arduino.h"
#include "main.hpp"

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