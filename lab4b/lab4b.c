//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <poll.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>


#include "mraa.h"


#define celsius 0
#define fahrenheit 1

// This temperature conversion function is lightly adapted from
// code appearing at http://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
float convert_temperature(int raw_reading, int scale){
    const int B = 4275;
    const int R0 = 100000;  

    float R = 1023.0/raw_reading-1.0;
    R = R0*R;
    float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15; 
    
    if(scale == celsius)
        return temp;
    return (temp * 9/5) + 32; // Fahrenheit conversion, if needed
}

static struct option long_options[] = {
    {"period", required_argument, 0, 'p'},
    {"scale", required_argument, 0, 's'},
    {"log", required_argument, 0, 'l'},
    {0,0,0,0}
};

int main(int argc, char** argv){
    int period = 10;
    int scale = 0;
    char* out_file_name = 0;
    int log_fd;
    int exit_status = 0;
    //Read in options
    int c;
    int option_index = 0;
    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1){
            break;
        }
        int temp;
        switch(c){
            case 'p':
                temp = atoi(optarg);
                if(temp <= 0){
                    fprintf(stderr, "Zero or Negative timeperiod specified\n");
                    exit_status = 1;
                    break;
                }
                period = temp;
                break;
            case 's':
                if (strlen(optarg) > 1){
                    fprintf(stderr, "Unrecognized temperature scale\n");
                    exit_status = 1;
                    break;
                }
                switch(optarg[0]){
                    case 'C':
                        scale = celsius;
                        break;
                    case 'F':
                        scale = fahrenheit;
                        break;
                    default:
                        fprintf(stderr, "Unrecognized temperature scale\n");
                        exit_status = 1;
                        break;
                }
                break;
            case 'l':
                out_file_name = optarg;
                break;
            default:
                fprintf(stderr, "Unrecognized argument: %s", optarg);
                exit_status = 1;
                break;
        }
    }
    // Open the provided log file as output (if it exists)
    if(out_file_name){
        log_fd = open(out_file_name, O_WRONLY | O_APPEND);
        if(log_fd == -1){
            fprintf(stderr, "Unable to open the stated log file: %s", strerror(errno));
            exit_status = 2;
            log_fd = STDOUT_FILENO;
        }
    }else{
        log_fd = STDOUT_FILENO;
    }
    //Initialize the GPIO switch
    mraa_aio_context mraa_aio_ptr = mraa_aio_init(1);
    if(mraa_aio_ptr == 0){
        fprintf(stderr, "Unable to initialize the Analog Input for the temperature sensor\n");
        fflush(stdout);
        exit_status = 2;
    }
    int a = mraa_aio_read(mraa_aio_ptr);
    fprintf(stdout, "Temperature sensor states %f\n", convert_temperature(a, scale));

    return exit_status;
}