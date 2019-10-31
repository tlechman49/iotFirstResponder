// Task for running wifi commands

#include "wifi_task.hpp"
#include "main.hpp"
#include "Arduino.h"
#include "string.h"
#include "WiFi.h"

char wifi_task::_ssid[32] = "";
char wifi_task::_pwd[64] = "";
IPAddress wifi_task::_ip(0,0,0,0);
int wifi_task::_isIpStatic = 0;

// Lines 18 to 20 are Static IP configurations
IPAddress static_ip(192,168,1,12);
const IPAddress gateway(192,168,1,1);
const IPAddress subnet(255,255,255,0);

IPAddress wifi_task::host_ip(192,168,1,1); 
WiFiClient wifi_task::client;
char wifi_task::_readMessage[32] = "";  // message read from TCP server
char wifi_task::_writeMessage[32] = ""; // message sent to TCP server
String tempString = ""; // used in reading data from TCP server

void TaskWiFi(void *pvParameters) 
{
    (void)pvParameters;
    uint32_t ulNotifiedValue;
    uint32_t ulRetVal = 0;

    wifi_task wifiTask;
  
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
            wifiTask.tcpClient();
        }

        if( ( ulNotifiedValue & 0x04 ) != 0 )
        {
            /* Bit 2 was set - process whichever event is represented by bit 2. */
            wifiTask.transmit();
        }

        if( ( ulNotifiedValue & 0x08 ) != 0 )
        {
            /* Bit 3 was set - process whichever event is represented by bit 3. */
            wifiTask.receive();
        }

        // Set return notification to the return value, 0 == SUCCESS
        xTaskNotify( taskHandleCLI, ulRetVal, eSetBits );
    }
}

int notifyWiFiAndWait(uint32_t notifyValue, uint32_t * ulNotifiedValue, TickType_t xTicksToWait)
{
    int u8RetVal = 0;

    u8RetVal += xTaskNotify( taskHandleWiFi, notifyValue, eSetBits );
    u8RetVal += xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                                 ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                 ulNotifiedValue, /* Notified value pass out in
                                                      ulNotifiedValue. */
                                 xTicksToWait );  /* Block based on ticks to wait */
    return u8RetVal;
}

wifi_task::wifi_task(){
    strcpy(_ssid, "");
    strcpy(_pwd, "");

    strcpy(_readMessage, "");
    strcpy(_writeMessage, "");
}

wifi_task::~wifi_task(){
    strcpy(_ssid, "");
    strcpy(_pwd, "");

    strcpy(_readMessage, "");
    strcpy(_writeMessage, "");
}

int wifi_task::connect()
{
    int connectTimeout = 0;

    //is true if IP is set to static in the CLI otherwise DHCP is used
    if (_isIpStatic)
    {
        setIpFromChipId(); // use the chip IDs to create unique IP addresses (hopefully our chips dont have similar internal MACs)
        WiFi.config(static_ip,gateway,subnet);
    }
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

int wifi_task::setIpStatic(int set)
{
    _isIpStatic = set;
    if (_isIpStatic == set) return 0; // Success
    else return 1; // failure
}

int wifi_task::tcpClient()  // establishes TCP client role
{
    int connectTimeout = 0;

    client.connect(host_ip,5005);   // connect to TCP server (Raspberry Pi)
    while ((!client.connected()) && connectTimeout < 100) // 10 seconds
    {
        vTaskDelay(100);
        connectTimeout++;
    }
    if (client.connected())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int wifi_task::transmit() // sends data to TCP server
{
    if (client.connected())
    {
        client.println(_writeMessage);
        return 0;
    }
    else
    {
        return 1;
    }
}

int wifi_task::receive()    // receives data from TCP server
{
    if (client.available())
    {
        tempString = client.readString();
        strcpy(_readMessage, tempString.c_str());
        return 0;
    }
    else 
    {
        return 1;
    }
}

int wifi_task::setMessage(const char* writeMessage) // allows user to write data to be sent in wifi_task.transmit()
{
    strlcpy(_writeMessage, writeMessage, 32); 
    if (strncmp(writeMessage, _writeMessage, 32) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void wifi_task::getMessage(char* readMessage)   // displays data received from wifi_task.receive()
{
    strlcpy(readMessage, _readMessage, 32); 
}

void wifi_task::setIpFromChipId()
{
    uint8_t chipid[6];

    esp_efuse_read_mac(chipid);

    static_ip[3] = chipid[5];
}
