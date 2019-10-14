#include "Arduino.h"
#include "CLI.hpp"
#include "main.hpp"
#include "wifi_task.hpp"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// set task handles to null
TaskHandle_t taskHandleCLI = NULL;
TaskHandle_t taskHandleWiFi = NULL;

// the setup function runs once when you press reset or power the board
void setup()
{
    // initializes the console
    initialize_console();

    // Create some initial tasks here
    xTaskCreatePinnedToCore(
        TaskWiFi, "TaskWiFi" // A name just for humans
        ,
        4096 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 3 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &taskHandleWiFi, ARDUINO_RUNNING_CORE);
  
    xTaskCreatePinnedToCore(
        TaskCLI, "TaskCLI"
        , 
        4096 // Stack size
        ,
        NULL, 1 // Priority
        ,
        &taskHandleCLI, ARDUINO_RUNNING_CORE);
  
    // Now the task scheduler, which takes over control of scheduling individual tasks, 
    // is automatically started.
}

void loop()
{
    // Empty. Things are done in Tasks.
}