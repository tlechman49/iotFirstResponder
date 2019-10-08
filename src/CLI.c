// CLI interface for debugging and testing on ESP32
// Scroll to "register_commands" function to start adding more commands

#include "CLI.h"
#include <stdio.h>
#include "esp_system.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "cmd_system.h"
#include "esp_vfs_dev.h"
#include "argtable3/argtable3.h"
#include "Arduino.h"

#include "wifi_connect.h"

// Initializes the CLI interface using the C style ESP32 IDF setup instead of Arduino serial
void initialize_console()
{
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .use_ref_tick = true
    };
    ESP_ERROR_CHECK( uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );

    // /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif

    register_commands();
}

// The task function for the CLI. 
// This task should run continuously at a low priority in order to process requests as they come
void TaskCLI(void *pvParameters) // This is a task.
{
    (void)pvParameters;

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = "esp32> ";

    printf("\n"
           "Hello!\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n");

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
    }

    for (;;)
    {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}

// ADD NEW COMMANDS AND REGISTER THEM HERE
// First, create the command and the register function using the syntax of foo below
// Second, include those function prototypes in the CLI.h header file
// Last, add the register function to this function
void register_commands()
{
    esp_console_register_help_command();
    register_system();

    //add new commands here
    reg_get_foo();

    //GPIO
    reg_digital_read();
    reg_analog_read();
    reg_digital_write();
    reg_analog_write();

    //Wifi Configuration
    reg_get_wificonect();
}

// Example command get_foo
int get_foo(int argc, char **argv)
{
    // we use printf to print to serial because of the extra functionality provided by the IDF
    printf("bar\r\n"); 
    return 0; // 0 = SUCCESS, 1 = FAILURE
}

// Example of registering get foo
void reg_get_foo(void)
{
    const esp_console_cmd_t cmd = {
        .command = "foo",
        .help = "Prints bar",
        .hint = NULL,
        .func = &get_foo,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

//digital read a gpio pin specified with an argument
int digital_read(int argc, char **argv)
{
    int val = 0;
    int nerrors = arg_parse(argc, argv, (void **) &d_read_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, d_read_args.end, argv[0]);
        return 1;
    }
    int pinOpts[14] = {4, 12, 13, 14, 15, 21, 25, 26, 27, 32, 33, 34, 36, 39}; // valid digital read pins
    int pinNum = d_read_args.pin_num->ival[0];
    for (int i = 0; i < 14; i++)
    {
        if (pinOpts[i] == pinNum)
        {
            pinMode(pinNum, INPUT);
            val = digitalRead(pinNum);
            printf("Pin %d = %d\r\n", pinNum, val); 
            return 0; // 0 = SUCCESS, 1 = FAILURE
        }
    }
    printf("Invalid pin number\r\n");
    return 1;
}

// register digital read to the cli
void reg_digital_read(void)
{
    d_read_args.pin_num =
        arg_int1("p", "pin", "[4 12 13 14 15 21 25 26 27 32 33 34 36 39]", "Read pin with given number");
    d_read_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "digital_read",
        .help = "Reads a specified pin",
        .hint = NULL,
        .func = &digital_read,
        .argtable = &d_read_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// analog read an analog pin specified with an argument
int analog_read(int argc, char **argv)
{
    uint16_t val = 0;
    int nerrors = arg_parse(argc, argv, (void **) &a_read_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, a_read_args.end, argv[0]);
        return 1;
    }
    int pinOpts[14] = {4, 12, 13, 14, 15, 25, 26, 27, 32, 33, 34, 35, 36, 39}; // valid analog read pins
    int pinNum = a_read_args.pin_num->ival[0];
    for (int i = 0; i < 14; i++)
    {
        if (pinOpts[i] == pinNum)
        {
            pinMode(pinNum, INPUT);
            val = analogRead(pinNum);
            printf("Pin %d = %d\r\n", pinNum, val); 
            return 0; // 0 = SUCCESS, 1 = FAILURE
        }
    }
    printf("Invalid pin number\r\n");
    return 1;
}

// register analog read to the cli
void reg_analog_read(void)
{
    a_read_args.pin_num =
        arg_int1("p", "pin", "[4 12 13 14 15 25 26 27 32 33 34 35 36 39]", "Read analog pin with given number");
    a_read_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "analog_read",
        .help = "Reads a specified analog pin. Returns X -> V = X*3.3/4095 (the A/D conversion is nonlinear but there are methods to improve the accuracy online",
        .hint = NULL,
        .func = &analog_read,
        .argtable = &a_read_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

//digital write a gpio pin specified with an argument
int digital_write(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &d_write_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, d_write_args.end, argv[0]);
        return 1;
    }
    int pinOpts[11] = {4, 12, 13, 14, 15, 21, 25, 26, 27, 32, 33}; // valid digital write pins
    int pinNum = d_write_args.pin_num->ival[0];
    for (int i = 0; i < 11; i++)
    {
        if (pinOpts[i] == pinNum)
        {
            int set = d_write_args.pin_set->ival[0];
            if (set == 0 || set == 1) // check if writing a valid value (on/off)
            {
                pinMode(pinNum, OUTPUT);
                digitalWrite(pinNum, set);
                printf("Pin %d set to %d\r\n", pinNum, set); 
                return 0; // 0 = SUCCESS, 1 = FAILURE
            }
            else
            {
                printf("Invalid set number\r\n");
                return 1;
            }
            
        }
    }
    printf("Invalid pin number\r\n");
    return 1;
}

// register digital write to the cli
void reg_digital_write(void)
{
    d_write_args.pin_num =
        arg_int1("p", "pin", "[4 12 13 14 15 21 25 26 27 32 33]", "Write pin with given number");
    d_write_args.pin_set = 
        arg_int1("s", "set", "<0|1>", "Sets the pin to a given level");
    d_write_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "digital_write",
        .help = "Write a specified pin to a given level",
        .hint = NULL,
        .func = &digital_write,
        .argtable = &d_write_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// analog write an analog pin specified with an argument to a given value
int analog_write(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &a_write_args);
    if (nerrors != 0) 
    {
        arg_print_errors(stderr, a_write_args.end, argv[0]);
        return 1;
    }
    int pinNum = a_write_args.pin_num->ival[0];
    if (pinNum == 25 || pinNum == 26) // if pinNum is a valid analog output pin
    {
        int set = a_write_args.pin_set->ival[0];
        if (set >= 0 && set <= 255) // if the value entered is a value between 0 and 255 to be sent to the dac
        {
            pinMode(pinNum, OUTPUT);
            dacWrite(pinNum, set);
            printf("Pin %d set to %d\r\n", pinNum, set); 
            return 0; // 0 = SUCCESS, 1 = FAILURE
        }
        else 
        {
            printf("Invalid set number\r\n");
            return 1;
        }
    }
    else 
    {
        printf("Invalid pin number\r\n");
        return 1;
    }
}

// register analog write to the cli
void reg_analog_write(void)
{
    a_write_args.pin_num =
        arg_int1("p", "pin", "25|26", "Write pin with given number");
    a_write_args.pin_set = 
        arg_int1("s", "set", "<0:255>", "Sets the pin to a given level");
    a_write_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "analog_write",
        .help = "Writes a specified pin. Given input X: V = 3.3*X/255",
        .hint = NULL,
        .func = &analog_write,
        .argtable = &a_write_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}