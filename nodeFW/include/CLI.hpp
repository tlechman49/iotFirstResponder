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

// GPIO commands
int digital_read(int argc, char **argv);
void reg_digital_read(void);

int analog_read_adj(int argc, char **argv);
void reg_analog_read_adj(void);

int analog_read(int argc, char **argv);
void reg_analog_read(void);

int digital_write(int argc, char **argv);
void reg_digital_write(void);

int analog_write(int argc, char **argv);
void reg_analog_write(void);

int wifi(int argc, char **argv);
void reg_wifi(void);

int cli_get_temp(int argc, char **argv);
void reg_get_temp(void);

int cli_get_co2(int argc, char **argv);
void reg_get_co2(void);

int cli_get_flame(int argc, char **argv);
void reg_get_flame(void);

int cli_demo(int argc, char **argv);
void reg_demo(void);

int onboard_led(int argc, char **argv);
void reg_onboard_led(void);

#ifdef __cplusplus
}
#endif