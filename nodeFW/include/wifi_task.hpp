#pragma once

#include "WiFi.h"

//defines for task notifications
#define CONNECT_WIFI    0x01
#define ESTABLISH_TCP   0x02
#define TRANSMIT_TCP    0x04
#define RECEIVE_TCP     0x08
#define FROM_CLI        0x10
#define FROM_SENSOR     0x20

void TaskWiFi(void *pvParameters);

int notifyWiFiAndWait(uint32_t notifyValue, uint32_t * ulNotifiedValue, TickType_t xTicksToWait);

class wifi_task
{
    public:
        wifi_task();
        ~wifi_task();

        int connect();

        static void getSSID(char* ssid);
        static int setSSID(const char* ssid);
        static void getPwd(char* pwd);
        static int setPwd(const char* pwd);
        static IPAddress getIP();
        static int setIpStatic(int set);

        int tcpClient();
        int transmit();
        int receive();
        static int setMessage(const char* writeMessage);
        static void getMessage(char* readMessage);
        static char _readMessage[32];
        static WiFiClient client;

        // int parseReceivedData();

    private:
        static char _ssid[32];
        static char _pwd[64];
        static IPAddress _ip;
        static int _isIpStatic;

        static IPAddress host_ip;
        static char _writeMessage[32];

        void setIpFromChipId();

};

void createTcpRecieve(wifi_task * wifiTask);
void TaskTcpReceive(void *pvParameters);
