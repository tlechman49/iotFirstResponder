#include "Adafruit_CCS811.h"
#include "argtable3/argtable3.h"    
#include "esp_console.h"

Adafruit_CCS811 ccs;

uint16_t get_co2(){
    //things to do: add include literal argument -i for initalize -g get shit... cool! 
    if(!ccs.begin()){
        while(1);
    }
    while(!ccs.available());
    if(ccs.available()){
        if(!ccs.readData()){
            uint16_t co2_level = ccs.geteCO2();
            return co2_level;
        }


    }
    else{
        return -1;
    }
}


//CLI command 2 get co2 levels 
int cli_get_co2(int argc, char **argv)
{
    uint16_t co2_level = get_co2();
    printf("%u\r\n",co2_level);
    return 0;
}

// register get co2
void reg_get_co2(void)
{
    const esp_console_cmd_t cmd = {
        .command = "get_co2",
        .help = "Reads data from c02 sensor",
        .hint = NULL,
        .func = &cli_get_co2,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}