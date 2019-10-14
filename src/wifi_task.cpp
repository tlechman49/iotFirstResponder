// Task for running wifi commands

#include "wifi_task.hpp"
#include "main.hpp"
#include "Arduino.h"
#include "string.h"
#include "WiFi.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

char wifi_task::_ssid[32] = "";
char wifi_task::_pwd[64] = "";
IPAddress wifi_task::_ip;

void TaskWiFi(void *pvParameters) 
{
    (void)pvParameters;
    uint32_t ulNotifiedValue;
    uint32_t ulRetVal = 0;

    wifi_task wifiTask;

    pinMode(LED_BUILTIN, OUTPUT);
  
    for (;;) // A Task shall never return or exit.
    {
        // Bits in this RTOS task's notification value are set by the notifying
        // tasks and interrupts to indicate which events have occurred. */
        xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         ULONG_MAX, /* Reset the notification value to 0 on exit. */
                         &ulNotifiedValue, /* Notified value pass out in
                                              ulNotifiedValue. */
                         portMAX_DELAY );  /* Block indefinitely. */

        /* Process any events that have been latched in the notified value. */

        if( ( ulNotifiedValue & 0x01 ) != 0 )
        {
            /* Bit 0 was set - process whichever event is represented by bit 0. */
            wifiTask.connect();
        }

        if( ( ulNotifiedValue & 0x02 ) != 0 )
        {
            /* Bit 1 was set - process whichever event is represented by bit 1. */
            ;
        }

        if( ( ulNotifiedValue & 0x04 ) != 0 )
        {
            /* Bit 2 was set - process whichever event is represented by bit 2. */
            ;
        }

        // Set return notification to the return value, 0 == SUCCESS
        xTaskNotify( taskHandleCLI, ulRetVal, eSetBits );
    }
}

wifi_task::wifi_task(){
    strcpy(_ssid, "");
    strcpy(_pwd, "");
}

wifi_task::~wifi_task(){
    strcpy(_ssid, "");
    strcpy(_pwd, "");
}

int wifi_task::connect()
{
    int connectTimeout = 0;

    WiFi.begin(_ssid, _pwd);
    while ((WiFi.status() != WL_CONNECTED) && connectTimeout < 100) // 10 seconds
    {
        vTaskDelay(100);
        connectTimeout++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        _ip = WiFi.localIP();
        return 0;
    }
    else
    {
        return 1;
    }
}

void wifi_task::getSSID(char* ssid)
{
    strlcpy(ssid, _ssid, 32); //32 is the max length of an ssid from the wifi standard
}

int wifi_task::setSSID(const char* ssid)
{
    strlcpy(_ssid, ssid, 32); //32 is the max length of an ssid from the wifi standard
    if (strncmp(ssid, _ssid, 32) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void wifi_task::getPwd(char* pwd)
{
    strlcpy(pwd, _pwd, 64); //64 is the max length of a pwd from the wifi standard
}

int wifi_task::setPwd(const char* pwd)
{
    strlcpy(_pwd, pwd, 64); //64 is the max length of a pwd from the wifi standard
    if (strncmp(pwd, _pwd, 64) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

IPAddress wifi_task::getIP()
{
    return _ip;
}
