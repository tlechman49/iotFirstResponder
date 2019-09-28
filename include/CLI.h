// CLI interface for debugging and testing on ESP32
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_ESP_CONSOLE_UART_NUM CONFIG_CONSOLE_UART_NUM
#define CONFIG_ESP_CONSOLE_UART_BAUDRATE 115200

void initialize_console();
void TaskCLI(void *pvParameters);
void register_commands();

// New commands
int get_foo(int argc, char **argv);
void reg_get_foo(void);

#ifdef __cplusplus
}
#endif