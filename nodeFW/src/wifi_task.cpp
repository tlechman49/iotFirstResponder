// Task for running wifi commands

#include "wifi_task.hpp"
#include "main.hpp"
#include "Arduino.h"
#include "string.h"
#include "WiFi.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_WIFI_CORE 0
#else
#define ARDUINO_WIFI_CORE 1
#endif

char wifi_task::_ssid[32] = "";
char wifi_task::_pwd[64] = "";
IPAddress wifi_task::_ip(0,0,0,0);
int wifi_task::_isIpStatic = 0;

// Static IP configurations
IPAddress static_ip(192,168,1,12);
const IPAddress gateway(192,168,1,1);
const IPAddress subnet(255,255,255,0);

IPAddress wifi_task::host_ip(192,168,86,76); 
#define TCP_PORT 5005

WiFiClient wifi_task::client;
char wifi_task::_readMessage[32] = "";  // message read from TCP server
char wifi_task::_writeMessage[32] = ""; // message sent to TCP server

void TaskWiFi(void *pvParameters) 
{
    (void)pvParameters;
    uint32_t ulNotifiedValue;
    uint32_t ulRetVal = 0;
    TaskHandle_t fromTask;

    wifi_task wifiTask;
  
    for (;;) // A Task shall never return or exit.
    {
        fromTask = NULL;
        // Bits in this RTOS task's notification value are set by the notifying
        // tasks and interrupts to indicate which events have occurred. */
        xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         ULONG_MAX, /* Reset the notification value to 0 on exit. */
                         &ulNotifiedValue, /* Notified value pass out in
                                              ulNotifiedValue. */
                         portMAX_DELAY );  /* Block indefinitely. */

        /* Process any events that have been latched in the notified value. */

        if( ( ulNotifiedValue & CONNECT_WIFI ) != 0 )
        {
            /* Bit 0 was set - process whichever event is represented by bit 0. */
            ulRetVal += wifiTask.connect();
        }

        if( ( ulNotifiedValue & ESTABLISH_TCP ) != 0 )
        {
            /* Bit 1 was set - process whichever event is represented by bit 1. */
            ulRetVal += wifiTask.tcpClient();
            if (ulRetVal == 0)
            {
                createTcpRecieve(&wifiTask);
            }
        }

        if( ( ulNotifiedValue & TRANSMIT_TCP ) != 0 )
        {
            /* Bit 2 was set - process whichever event is represented by bit 2. */
            ulRetVal += wifiTask.transmit();
        }

        if( ( ulNotifiedValue & RECEIVE_TCP ) != 0 )
        {
            /* Bit 3 was set - process whichever event is represented by bit 3. */
            ulRetVal += wifiTask.receive();
        }

        if( ( ulNotifiedValue & FROM_CLI ) != 0 )
        {
            /* Bit 4 was set - process whichever event is represented by bit 4. */
            fromTask = taskHandleCLI;
        }

        if( ( ulNotifiedValue & FROM_SENSOR ) != 0 )
        {
            /* Bit 5 was set - process whichever event is represented by bit 5. */
            fromTask = taskHandleSensor;
        }     

        if (fromTask != NULL)
        {
            // Set return notification to the return value, 0 == SUCCESS
            xTaskNotify( fromTask, ulRetVal, eSetBits );
        }
    }
}

int notifyWiFiAndWait(uint32_t notifyValue, uint32_t * ulNotifiedValue, TickType_t xTicksToWait)
{
    xTaskNotify( taskHandleWiFi, notifyValue, eSetBits );
    if (pdPASS == xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                                 ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                 ulNotifiedValue, /* Notified value pass out in
                                                      ulNotifiedValue. */
                                 xTicksToWait ))  /* Block based on ticks to wait */
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void createTcpRecieve(wifi_task * wifiTaskPtr)
{
    xTaskCreatePinnedToCore(
        TaskTcpReceive, "TaskTcpReceive" // A name just for humans
        ,
        2048 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        wifiTaskPtr //pvParameter set to pointer to the wifiTask class
        , 
        1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &taskHandleTcpReceive, ARDUINO_WIFI_CORE);
}

// check tcp receive buffer every 50ms and write what is received into the _readMessage char array
void TaskTcpReceive(void *pvParameters) 
{
    wifi_task * wifiTask = (wifi_task *)pvParameters;
    String tempString = "";

    for (;;) // A Task shall never return or exit.
    {
        vTaskDelay(50);
        if (wifiTask->client.available())
        {
            // printf("message received!\r\n");
            tempString = wifiTask->client.readString();
            strcpy(wifiTask->_readMessage, tempString.c_str());
            xTaskNotify( taskHandleOutputManager, 0, eSetValueWithOverwrite );
        }
    }
}

wifi_task::wifi_task(){
    strcpy(_ssid, "mrwifi");
    strcpy(_pwd, "Bananas321");

    strcpy(_readMessage, "");
    strcpy(_writeMessage, "");

    vTaskDelay(100);
    if (connect() == 0) // if connected to wifi
    {
        vTaskDelay(100);
        if (tcpClient() == 0) // if connected to tcp
        {
            createTcpRecieve(this); // start the tcp receive task
            xTaskNotify( taskHandleSensor, 0x00, eSetBits ); // start the demo mode
        }
    }
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

    client.connect(host_ip, TCP_PORT);   // connect to TCP server (Raspberry Pi)
    while ((!client.connected()) && connectTimeout < 100) // 10 seconds
    {
        vTaskDelay(100);
        connectTimeout++;
    }
    if (client.connected())
    {
        // once a node is connected, send over a piece of its MAC address
        uint8_t chipid[6];
        esp_efuse_read_mac(chipid);
        client.print(chipid[5]);
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
        client.print(_writeMessage);
        return 0;
    }
    else
    {
        return 1;
    }
}

int wifi_task::receive()    // receives data from TCP server
{
    String tempString = "";
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

// int wifi_task::parseReceivedData()
// {
//     // printf("parsing message\r\n");
//     uint32_t period = 0;
//     sscanf(_readMessage, "%u", &period);
//     // printf("message scanned\r\n");

//     xTaskNotify( taskHandleOnboardLed, period, eSetValueWithOverwrite );
//     // printf("led notified\r\n");

//     return 0;
// }
