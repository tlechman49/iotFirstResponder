#pragma once

#include "WiFi.h"

void TaskWiFi(void *pvParameters);

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
    private:
        static char _ssid[32];
        static char _pwd[64];
        static IPAddress _ip;
        static int _isIpStatic;

        static IPAddress host_ip;
        static WiFiClient client;
        static char _readMessage[32];
        static char _writeMessage[32];
        String tempString;

};
