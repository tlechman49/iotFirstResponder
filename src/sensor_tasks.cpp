#include "Adafruit_CCS811.h"
#include "argtable3/argtable3.h"    
#include "esp_console.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"


Adafruit_CCS811 ccs;


//gets temperature from pin 36 only.. can be changed by changing the channel .. but this should be fine
double get_temp(){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
    int adcval = adc1_get_raw(ADC1_CHANNEL_0);
    double v = (adcval / 4095.0) * 1.1 ; //in volts
    double temp = (v-0.5)*100; // in celcius
    return temp;
}

int cli_get_temp(int argc, char **argv)
{
    double temp = get_temp();
    printf("Temp =  %f degrees C\r\n",temp);
    return 0;
}

void reg_get_temp(void)
{
    const esp_console_cmd_t cmd = {
        .command = "get_temp",
        .help = "Reads data from temp sensor on PIN 36 and converts to temperature",
        .hint = NULL,
        .func = &cli_get_temp,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

}
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


