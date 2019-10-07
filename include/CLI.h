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
struct {
    struct arg_int *pin_num;
    struct arg_end *end;
} d_read_args;
int digital_read(int argc, char **argv);
void reg_digital_read(void);
struct {
    struct arg_int *pin_num;
    struct arg_end *end;
} a_read_args;
int analog_read(int argc, char **argv);
void reg_analog_read(void);

struct {
    struct arg_int *pin_num;
    struct arg_int *pin_set;
    struct arg_end *end;
} d_write_args;
int digital_write(int argc, char **argv);
void reg_digital_write(void);
struct {
    struct arg_int *pin_num;
    struct arg_int *pin_set;
    struct arg_end *end;
} a_write_args;
int analog_write(int argc, char **argv);
void reg_analog_write(void);

#ifdef __cplusplus
}
#endif