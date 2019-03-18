//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>


#include "mraa.h"

int main(){
    int results = 0;
    mraa_aio_context mraa_aio_ptr = mraa_aio_init(1);
    if(mraa_aio_ptr == 0){
        fprintf(stdout, "Unable to initialize the Analog Input for the temperature sensor in port 1\n");
        results = 1;
    }
    mraa_gpio_context mraa_gpio_ptr = mraa_gpio_init(60);
    if(mraa_gpio_ptr == 0){
        fprintf(stdout, "Unable to initialize the Digital Button Input in port 60\n");
        results = 1;
    }
    mraa_gpio_dir(mraa_gpio_ptr, MRAA_GPIO_IN); // set button to take input
    int button_read = mraa_gpio_read(mraa_gpio_ptr);
    if(button_read == -1){
        fprintf(stdout, "Unable to get a valid reading from the button\n");
        results = 1;
    }
    int temp_read = mraa_aio_read(mraa_aio_ptr);
    if(temp_read == -1){
        fprintf(stdout, "Unable to get a valid reading from the temperature sensor\n");
        results = 1;
    }
    // Close the open sensors
    if(mraa_gpio_ptr)
        mraa_gpio_close(mraa_gpio_ptr);
    if(mraa_aio_ptr)
        mraa_aio_close(mraa_aio_ptr);
        
    if(results){
        fprintf(stdout, "One or more checks failed\n");
    }else{
        fprintf(stdout, "All checks passed!\n");
    }
    fflush(stdout);
    return results;
}