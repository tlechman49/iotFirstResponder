// Wifi Configuration
// CLI implementation based out of foo CLI functions
// Wifi connection based out of ESP-IDF wifi template
// Static IP configuration confirmed on Raspberry Pi
// TCP/IP configurations are still a work in progress

// Remaining to-do's:
// Display some form on confirmation message when running the connect or disconnect functions
// Test printing IP address

#include "CLI.hpp"
#include "esp_console.h"

#include "wifi_connect.h"

#include "esp_wifi.h"
#include "tcpip_adapter.h"
#include "argtable3/argtable3.h"
#include "esp_event_loop.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

int get_wificonnect(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &wificonnect_args);
    if (nerrors != 0) 
    {
        arg_print_errors(stderr, wificonnect_args.end, argv[0]);
        return 1;
    }
    if (wificonnect_args.connect->count)
    {
        connect();
        return 0;
    }
    else if (wificonnect_args.disconnect->count)
    {
        disconnect();
        return 0; 
    }
    else if (wificonnect_args.ip->count)
    {
        print_ip();
        return 0;
    }
    else
    {
        return 1;
    }
}

void reg_get_wificonnect(void)
{
    wificonnect_args.connect =
        arg_lit0("c", "connect", "Connect to First Responder IoT network");
    wificonnect_args.disconnect =
        arg_lit0("d", "disconnect", "Disconnect from First Responder IoT network");
    wificonnect_args.ip =
        arg_lit0("i", "print_ip", "Print IP address");
    wificonnect_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "wificonfig",
        .help = "Provides options for generic wifi commands",
        .hint = NULL,
        .func = &get_wificonnect,
        .argtable = &wificonnect_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void connect(void)
{
    // Configure static IP address
    tcpip_adapter_init();
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    tcpip_adapter_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192, 168, 1,2);
    IP4_ADDR(&ipInfo.gw, 192, 168, 1, 1);
    IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);

    // Configure Wi-FI station settings
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "80kNetworkTest",
            .password = "80kpassword",
            .bssid_set = false
        }
    };

    // Connect to Wi-Fi as a station
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

void disconnect(void)
{
    tcpip_adapter_init();

    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "80kNetworkTest",
            .password = "80kpassword",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_stop() );
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
}

void print_ip(void)
{
    ;
}
