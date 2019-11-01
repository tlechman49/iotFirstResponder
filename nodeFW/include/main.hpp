#pragma once

#include "Arduino.h"

/* Handles for the tasks create by main(). */
extern TaskHandle_t taskHandleCLI, taskHandleWiFi, taskHandleSensor, taskHandleOnboardLed;