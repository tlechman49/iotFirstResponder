// Wifi Configuration
// CLI implementation based out of foo CLI functions
// Wifi connection based out of ESP-IDF wifi template
// TCP/IP configurations are still a work in progress

// Remaining to-do's:
// Display some form on confirmation message when running the connect or disconnect functions
// Test static IP configuration
// Test printing IP address

#include "CLI.h"
#include <stdio.h>
#include "esp_system.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "cmd_system.h"
#include "esp_vfs_dev.h"

#include "wifi_connect.h"

#include "FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "gpio.h"

#include "tcpip_adapter.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

int get_wificonnect(int argc, char **argv)
{
    
    printf("Please make a selection:\n\n1. Connect to First Responder IoT network\n2. Disconnect from First Responder IoT network\n3. Show IP address\n4. Return to CLI\n");

    while (true) {
        
        char* line = linenoise("Input selection: ");
        if (line == NULL) { /* Ignore empty lines */
            linenoiseFree(line);
            continue;
        } else if (*line == '1') {
            connect();
            linenoiseFree(line);
            break;
        } else if (*line == '2') {
            disconnect();
            linenoiseFree(line);
            break;
        } else if (*line == '3') {
            print_ip();
            linenoiseFree(line);
            break;
        } else if (*line == '4') {
            linenoiseFree(line);
            break;
        } else {
            printf("Unrecognized command, please try again\n");
            linenoiseFree(line);
        }
           
    }

    return 0; // 0 = SUCCESS, 1 = FAILURE
}

void reg_get_wificonnect(void)
{
    const esp_console_cmd_t cmd = {
        .command = "wificonfig",
        .help = "Opens Wifi Connection menu.",
        .hint = NULL,
        .func = &get_wificonnect,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void connect(void)
{
    nvs_flash_init();

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

    nvs_flash_init();
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
    
}
