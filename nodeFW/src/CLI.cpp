// CLI interface for debugging and testing on ESP32
// Scroll to "register_commands" function to start adding more commands

#include "CLI.hpp"
#include <stdio.h>
#include "esp_system.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "cmd_system.h"
#include "esp_vfs_dev.h"
#include "argtable3/argtable3.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"
// #include "Arduino.h"
#include "wifi_connect.h"
#include "main.hpp"
#include "wifi_task.hpp"
#include "sensor_task.hpp"

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
    uart_config_t uart_config;
    uart_config.baud_rate       = CONFIG_ESP_CONSOLE_UART_BAUDRATE;
    uart_config.data_bits       = UART_DATA_8_BITS;
    uart_config.parity          = UART_PARITY_DISABLE;
    uart_config.stop_bits       = UART_STOP_BITS_1;
    uart_config.use_ref_tick    = true;
    ESP_ERROR_CHECK( uart_param_config((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );

    // /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config;
    console_config.max_cmdline_args = 8;
    console_config.max_cmdline_length = 256;
#if CONFIG_LOG_COLORS
        console_config.hint_color = atoi(LOG_COLOR_CYAN)
#endif
    
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

    // initializes the console
    initialize_console();

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
    reg_analog_read_adj();

    //Wifi Configuration
    reg_get_wificonnect();

    //Operate wifi task
    reg_wifi();

    //sensor tasks
    reg_get_co2();
    reg_get_temp();
    reg_get_flame();

    //demo
    reg_demo();

    //outputs
    reg_onboard_led();
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

struct {
    struct arg_int *pin_num;
    struct arg_end *end;
} d_read_args;

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

// corrected ADC read function designed for pin 36
int analog_read_adj(int argc, char **argv)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
    int val = adc1_get_raw(ADC1_CHANNEL_0);
    printf("%d\r\n",val);
    return 0;
}

//register the analog read adjustment to the CLI
void reg_analog_read_adj(void)
{
    const esp_console_cmd_t cmd = {
        .command = "analog_read_adj",
        .help = "Reads a full scale calibrated ADC input",
        .hint = NULL,
        .func = &analog_read_adj,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

struct {
    struct arg_int *pin_num;
    struct arg_end *end;
} a_read_args;

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

struct {
    struct arg_int *pin_num;
    struct arg_int *pin_set;
    struct arg_end *end;
} d_write_args;

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

struct {
    struct arg_int *pin_num;
    struct arg_int *pin_set;
    struct arg_end *end;
} a_write_args;

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

struct {
    struct arg_str *ssid;
    struct arg_str *pwd;
    struct arg_lit *connect;
    struct arg_lit *ip;
    struct arg_lit *staticIp;
    struct arg_lit *tcpClient;
    struct arg_lit *transmit;
    struct arg_str *write;
    struct arg_lit *receive;
    struct arg_lit *display;
    struct arg_end *end;
} wifi_args;

// sends messages and changes statics in wifi task to perform operations
int wifi(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &wifi_args);
    if (nerrors != 0) 
    {
        arg_print_errors(stderr, wifi_args.end, argv[0]);
        return 1;
    }

    uint32_t ulNotifiedValue;
    int u8RetVal = 0;

    // changes static ssid 
    if (wifi_args.ssid->count)
    {
        char ssid[32];
        const char *ssidIn = wifi_args.ssid->sval[0];
        u8RetVal += wifi_task::setSSID(ssidIn);
        wifi_task::getSSID(ssid);
        printf("\r\nssid: ");
        printf(ssid);
        printf("\r\n");
    }

    // changes static pwd
    if (wifi_args.pwd->count)
    {
        char pwd[64];
        const char *pwdIn = wifi_args.pwd->sval[0];
        u8RetVal += wifi_task::setPwd(pwdIn);
        wifi_task::getPwd(pwd);
        printf("\r\npwd: ");
        printf(pwd);
        printf("\r\n");
    }

    // changes static boolean to inform wifi task to connect with static ip
    if (wifi_args.staticIp->count)
    {
        u8RetVal += wifi_task::setIpStatic(1);
        printf("\r\nStatic IP set\r\n");
    }

    // informs wifi task to begin connection procedure
    if (wifi_args.connect->count)
    {
        if (notifyWiFiAndWait((FROM_CLI | CONNECT_WIFI), &ulNotifiedValue, portMAX_DELAY))   return 1;
        u8RetVal += ulNotifiedValue;
    }

    // prints the ip address currently in use by the wifi task
    if (wifi_args.ip->count)
    {
        printf(wifi_task::getIP().toString().c_str());
        printf("\r\n");
    }

    if (wifi_args.tcpClient->count)
    {
        if (notifyWiFiAndWait((FROM_CLI | ESTABLISH_TCP), &ulNotifiedValue, portMAX_DELAY))   return 1;
        u8RetVal += ulNotifiedValue;
    }

    if (wifi_args.transmit->count)
    {
        if (notifyWiFiAndWait((FROM_CLI | TRANSMIT_TCP), &ulNotifiedValue, portMAX_DELAY))   return 1;
        u8RetVal += ulNotifiedValue;
    }

    if (wifi_args.receive->count)
    {
        if (notifyWiFiAndWait((FROM_CLI | RECEIVE_TCP), &ulNotifiedValue, portMAX_DELAY))   return 1;
        u8RetVal += ulNotifiedValue;
    }

    if (wifi_args.write->count)
    {
        const char *messageIn = wifi_args.write->sval[0];
        u8RetVal += wifi_task::setMessage(messageIn);
    }

    if (wifi_args.display->count)
    {
        char readMessage[32];
        wifi_task::getMessage(readMessage);
        printf(readMessage);
        printf("\r\n");
    }
    return u8RetVal;
}

// registers the wifi commands to the cli
void reg_wifi(void)
{
    wifi_args.ssid =
        arg_str0("s", "ssid", "<ssid>", "SSID used for wifi connections");
    wifi_args.pwd =
        arg_str0("p", "pwd", "<pwd>", "Password used for wifi connections");
    wifi_args.connect =
        arg_lit0("c", "connect", "Connect to First Responder IoT network");
    wifi_args.ip =
        arg_lit0("i", "print_ip", "Print IP address");
    wifi_args.staticIp = 
        arg_lit0("a", "static_ip", "Sets the IP to static");
    wifi_args.tcpClient = 
        arg_lit0("e", "establish_tcp", "Establish TCP connection with the TCP Server");
    wifi_args.transmit = 
        arg_lit0("t", "tcp_transmit", "Transmit data to TCP Server");
    wifi_args.write = 
        arg_str0("w", "write_message", "<message>", "Write message to be sent over TCP");
    wifi_args.receive = 
        arg_lit0("r", "tcp_receive", "Receive data from TCP Server");
    wifi_args.display =
        arg_lit0("d", "display_message", "Display message read over TCP");
    wifi_args.end = arg_end(10);

    const esp_console_cmd_t cmd = {
        .command = "wifi",
        .help = "Provides options for generic wifi commands",
        .hint = NULL,
        .func = &wifi,
        .argtable = &wifi_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// cli command to get the temperature
int cli_get_temp(int argc, char **argv)
{
    TMP3X tmp;
    if (tmp.readTemp() == 0)
    {
        double temp = tmp.getTemp();
        printf("Temp =  %f degrees C\r\n", temp);
        return 0;
    }
    else
    {
        return 1;
    }
    
}

// register get temperature to the cli
void reg_get_temp(void)
{
    const esp_console_cmd_t cmd = {
        .command = "get_temp",
        .help = "Reads data from temp sensor on PIN 36 and converts to temperature",
        .hint = NULL,
        .func = &cli_get_temp,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


//CLI command to get co2 levels

struct {
    struct arg_lit *begin;
    struct arg_end *end;
} co2_args;


//things to do: add include literal argument -i for initalize -g get shit... cool!
int cli_get_co2(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &co2_args);
    if (nerrors != 0) 
    {
        arg_print_errors(stderr, co2_args.end, argv[0]);
        return 1;
    }

    CCS_CO2 co2;

    // initialize co2 sensor
    if (co2_args.begin->count)
    {
        if (co2.begin())
        {
            return 1;
        }
    }

    // read sensor the print to cli
    if (co2.readCo2() == 0)
    {
        uint16_t co2_level = co2.getCo2();
        printf("%u\r\n", co2_level);
        return 0;
    }
    else
    {
        return 1;
    }
}

// register get co2 to the cli
void reg_get_co2(void)
{
    co2_args.begin =
        arg_lit0("b", "begin", "Begin sensor for the first time, initializes sensor");
    co2_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "get_co2",
        .help = "Reads data from c02 sensor",
        .hint = NULL,
        .func = &cli_get_co2,
        .argtable = &co2_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

// cli command to get the temperature
int cli_get_flame(int argc, char **argv)
{
    Flame flame;
    if (flame.readFlame() == 0)
    {
        int fl = flame.getFlame();
        printf("Flame =  %d\r\n", fl);
        return 0;
    }
    else
    {
        return 1;
    }
    
}

// register get flame to the cli
void reg_get_flame(void)
{
    const esp_console_cmd_t cmd = {
        .command = "get_flame",
        .help = "Reads data from flame sensor on PIN 39 and returns Boolean value",
        .hint = NULL,
        .func = &cli_get_flame,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

struct {
    struct arg_lit *wireless;
    struct arg_end *end;
} demo_args;

// demo features of the device for presentation
int cli_demo(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &demo_args);
    if (nerrors != 0) 
    {
        arg_print_errors(stderr, demo_args.end, argv[0]);
        return 1;
    }

    printf("\r\nDemo starting...\r\n");

    if (demo_args.wireless->count)
    {
        xTaskNotify( taskHandleSensor, 0x01, eSetBits );
    }
    else 
    {
        xTaskNotify( taskHandleSensor, 0x00, eSetBits );
    }

    return 0;
}

// register demo to the cli
void reg_demo(void)
{
    demo_args.wireless =
        arg_lit0("w", "wireless", "Send tcp data with fake sensor data");
    demo_args.end = arg_end(1);
    
    const esp_console_cmd_t cmd = {
        .command = "demo",
        .help = "**Must connect to TCP server first** Puts the device in demo mode to send sensor data automatically",
        .hint = NULL,
        .func = &cli_demo,
        .argtable = &demo_args,
};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

struct {
    struct arg_int *period;
    struct arg_end *end;
} onboard_led_args;

// start the onboard led blinking at a given period
int onboard_led(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &onboard_led_args);
    if (nerrors != 0) 
    {
        arg_print_errors(stderr, onboard_led_args.end, argv[0]);
        return 1;
    }

    uint32_t period = (uint32_t)onboard_led_args.period->ival[0];
    printf("Starting LED blink at period of %d ms\r\n", period);

    xTaskNotify( taskHandleOnboardLed, period, eSetValueWithOverwrite );
    
    return 0;
}

// register onboard led to the cli
void reg_onboard_led(void)
{
    onboard_led_args.period =
        arg_int1("p", "period", "", "Set period of onboard led blink");
    onboard_led_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "onboard_led",
        .help = "Blinks onboard led at specified period",
        .hint = NULL,
        .func = &onboard_led,
        .argtable = &onboard_led_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}