#include "output_tasks.hpp"
#include "Arduino.h"
#include "main.hpp"
#include "FastLED.h"
#include "string.h"
#include "wifi_task.hpp"
#include <stack>

#define LED_PIN 13
#define ON_TIME_MS 200

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// max number of tasks based on available pins 
#define MAX_OUTPUTS 17

TaskFunction_t taskFuncs[2] = {TaskOnboardLed, TaskLedStrip};

// parses commands to either start a new task or notify an ongoing task of a new command
void TaskOutputManager(void *pvParameters)
{    
    (void)pvParameters;

    std::stack <command_s> commandStack;
    char message[32];
    command_s tempCommand{(uint32_t) -1, 0, 0};
    outTask_s outTasks[MAX_OUTPUTS];
    int numberOfTasks = 0;
    uint32_t ulNotifiedValue;

    // iterate over toplevel array to complete each command
    // check if a task exists for that command and if it does not then create a task
    // send the task a pin number as the notification value and wait for a response
    // task responds when in records the pin number and then enters the loop where it blocks for a command
    // response triggers another response from the manager with the command 

    // if the task already exists in the table then just send a command
    for(;;)
    {
        // wait for a notification
        xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         ULONG_MAX, /* Reset the notification value to 0 on exit. */
                         &ulNotifiedValue, /* Notified value pass out in
                                              ulNotifiedValue. */
                         portMAX_DELAY );  /* Block indefinitely. */

        // capture message from the wifi task
        wifi_task::getMessage(message);

        // parse message into an array of commands
        char * singleCmd = strtok(message, ",");
        
        while( singleCmd != NULL ) 
        {
            // printf( " %s\r\n", singleCmd ); //printing each token
            if (3 == sscanf(singleCmd, "%u.%u.%u", &tempCommand.type, &tempCommand.pin, &tempCommand.msg))
            {
                commandStack.push(tempCommand);
            }
            singleCmd = strtok(NULL, ",");
        }
        // printf("Command stack has %u commands left\r\n", commandStack.size());
        // printf("Top command: %u.%u.%u\r\n", commandStack.top().type, commandStack.top().pin, commandStack.top().msg);
        while (commandStack.size() > 0)
        {
            if (numberOfTasks == 0)
            {
                createNewTask(commandStack.top(), &outTasks[numberOfTasks]);
                commandStack.pop();
                numberOfTasks++;
            }
            else
            {
                for (int i = 0; i < MAX_OUTPUTS; i++)
                {
                    // check if the task already exists, if it does send it a message
                    if ((commandStack.top().type == outTasks[i].type) && (commandStack.top().pin == outTasks[i].pin))
                    {
                        xTaskNotify(outTasks[i].handle, (uint32_t)commandStack.top().msg, eSetBits);
                        commandStack.pop();
                        break;
                    }
    
                    // if the task doesn't exist make a new task
                    if ((i == MAX_OUTPUTS-1) && (numberOfTasks < MAX_OUTPUTS))
                    {
                        createNewTask(commandStack.top(), &outTasks[numberOfTasks]);
                        commandStack.pop();
                        numberOfTasks++;
                    }
                }
            }   
            
        }
    }
}

// creates a new task and sends the tasks first command
int createNewTask(command_s cmd, outTask_s * outTask)
{
    outTask->type = cmd.type;
    outTask->pin = cmd.pin;
    // printf("stored type (%u) and pin (%u) to outTask list\r\n", outTask->type, outTask->pin);
    xTaskCreatePinnedToCore(
        taskFuncs[cmd.type], "outTask" // A name just for humans
        ,
        2048 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        (void *) cmd.pin, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &(outTask->handle), ARDUINO_RUNNING_CORE);
    xTaskNotify(outTask->handle, cmd.msg, eSetBits);
    // vTaskDelay(100);

    return 0;
}


// blinks onboard led based on the given period
// if period is less than 400ms, set it to 400ms
// if 0 stop the blinking
void TaskOnboardLed(void *pvParameters) 
{
    uint32_t pin;
    uint32_t ulNotifiedValue = 0;
    TickType_t offTime = portMAX_DELAY;

    pin = (uint32_t) pvParameters;
    pinMode(pin, OUTPUT);

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
        digitalWrite((uint8_t) pin, HIGH);
        vTaskDelay(ON_TIME_MS);
        digitalWrite((uint8_t) pin, LOW);
    }
}

// wipes an animation either away from or toward the connection point of the led strip
// sets the leds to black and goes into block state if the animation is stopped
void TaskLedStrip(void *pvParameters) 
{
    uint32_t pin = (uint32_t) pvParameters;
    uint32_t ulNotifiedValue = 0;
    TickType_t offTime = portMAX_DELAY;
    int dir = 0;
    LED led(pin);

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
    switch (pin) {
    case 4:
      FastLED.addLeds<LED_TYPE, 4, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 5:
      FastLED.addLeds<LED_TYPE, 5, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 12:
      FastLED.addLeds<LED_TYPE, 12, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 13:
      FastLED.addLeds<LED_TYPE, 13, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 14:
      FastLED.addLeds<LED_TYPE, 14, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 15:
      FastLED.addLeds<LED_TYPE, 15, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 16:
      FastLED.addLeds<LED_TYPE, 16, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 17:
      FastLED.addLeds<LED_TYPE, 17, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 18:
      FastLED.addLeds<LED_TYPE, 18, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 19:
      FastLED.addLeds<LED_TYPE, 19, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 21:
      FastLED.addLeds<LED_TYPE, 21, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 22:
      FastLED.addLeds<LED_TYPE, 22, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 23:
      FastLED.addLeds<LED_TYPE, 23, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 27:
      FastLED.addLeds<LED_TYPE, 27, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 32:
      FastLED.addLeds<LED_TYPE, 32, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    case 33:
      FastLED.addLeds<LED_TYPE, 33, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
      break;

    // case 34:
    //   FastLED.addLeds<LED_TYPE, 34, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    //   break;

    default:
      Serial.println("Unsupported Pin");
      break;
  }

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

