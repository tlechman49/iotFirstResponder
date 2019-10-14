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
    private:
        static char _ssid[32];
        static char _pwd[64];
        static IPAddress _ip;

};
