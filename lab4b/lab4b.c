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
#include <time.h>

#include "mraa.h"


#define celsius 0
#define fahrenheit 1
#define POLL_INTERVAL 1000
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

char* next_output(char* buf, float temp){
    time_t raw_time = time(0);
    struct tm* timestruct = localtime(&raw_time);
    sprintf(buf, "%02d:%02d:%02d %.01f\n", timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec, temp);
    return buf;
}
char* shutdown_message(char* buf){
    time_t raw_time = time(0);
    struct tm* timestruct = localtime(&raw_time);
    sprintf(buf, "%02d:%02d:%02d SHUTDOWN\n", timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
    return buf;
}

static struct option long_options[] = {
    {"period", required_argument, 0, 'p'},
    {"scale", required_argument, 0, 's'},
    {"log", required_argument, 0, 'l'},
    {0,0,0,0}
};

int main(int argc, char** argv){
    int period = 1;
    int scale = 0;
    char* out_file_name = 0;
    int log_fd = -1;
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
                fprintf(stderr, "Unrecognized argument");
                exit_status = 1;
                break;
        }
    }
    // Open the provided log file as output (if it exists)
    if(out_file_name){
        log_fd = open(out_file_name, O_WRONLY | O_APPEND | O_CREAT);
        if(log_fd == -1){
            fprintf(stderr, "Unable to open the stated log file: %s", strerror(errno));
            exit_status = 2;
            log_fd = STDOUT_FILENO;
        }
    }
    //Initialize the GPIO switch
    mraa_aio_context mraa_aio_ptr = mraa_aio_init(1);
    if(mraa_aio_ptr == 0){
        fprintf(stderr, "Unable to initialize the Analog Input for the temperature sensor\n");
        fflush(stdout);
        exit_status = 2;
    }
    mraa_gpio_context mraa_gpio_ptr = mraa_gpio_init(60);
    if(mraa_gpio_ptr == 0){
        fprintf(stderr, "Unable to initialize the Digital Button Input\n");
        fflush(stderr);
        exit_status=2;
    }
    mraa_gpio_dir(mraa_gpio_ptr, MRAA_GPIO_IN); // set button to take input

    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    time_t next_output_time = time(0);

    // Loop temperature and polling for input
    int raw_temp;
    char buf[30];
    char nextchar;
    char* input_buf = malloc(sizeof(char)* 100);
    int input_buf_size = 100;
    int input_buf_pos = 0;
    int reporting = 1;
    while(1){
        if(time(0) >= next_output_time && reporting){
            next_output_time = time(0) + period;
            //Produce report
            raw_temp = mraa_aio_read(mraa_aio_ptr);
            next_output(buf, convert_temperature(raw_temp, scale));
            write(STDOUT_FILENO, buf, strlen(buf));
            if(log_fd != -1){
                write(log_fd, buf, strlen(buf));
            }
        }
        poll(&pfd, 1, POLL_INTERVAL);
        // read and deal with input, if available
        if(pfd.revents & POLLIN){
            read(STDIN_FILENO, &nextchar, 1);
            if(nextchar == '\n'){
                //Process command
                if(strncmp("SCALE=F", input_buf, input_buf_pos) == 0){
                    scale = fahrenheit;
                }
                if(strncmp("SCALE=C", input_buf, input_buf_pos) == 0){
                    scale = celsius;
                }
                if(strncmp("PERIOD=", input_buf, 7) == 0){
                    if(input_buf_pos == input_buf_size) // this is an impossibly large number...
                        input_buf[input_buf_pos -1] = 0;
                    else
                        input_buf[input_buf_pos] = 0;
                    int newperiod = atoi(input_buf + 7);
                    if(newperiod > 0){
                        period = newperiod;
                    }
                }
                if(strncmp("START", input_buf, input_buf_pos) == 0){
                    reporting = 1;
                }
                if(strncmp("STOP", input_buf, input_buf_pos) == 0){
                    reporting = 0;
                }
                if(strncmp("LOG ", input_buf, 4) == 0){
                    write(STDOUT_FILENO, input_buf, input_buf_pos);
                    write(STDOUT_FILENO, "\n", 1);
                }
                if(strncmp("OFF", input_buf, input_buf_pos) == 0){
                    char sd_buf[30];
                    shutdown_message(sd_buf);
                    write(STDOUT_FILENO, sd_buf, strlen(sd_buf));
                    if(log_fd != -1){
                        write(log_fd, input_buf, input_buf_pos);
                        write(log_fd, "\n", 1);
                    }
                    if(log_fd != -1){
                        write(log_fd, sd_buf, strlen(sd_buf));
                    }
                    break;
                }
                // Log the command, if logging in enabled
                if(log_fd != -1){
                    write(log_fd, input_buf, input_buf_pos);
                    write(log_fd, "\n", 1);
                }
                input_buf_pos = 0;
            }else{
                if(input_buf_size == input_buf_pos){
                    // Resize input array
                    input_buf = realloc(input_buf, input_buf_size*2);
                    if(input_buf == 0){
                        fprintf(stderr, "Unable to allocate more memory: %s\n", strerror(errno));
                        exit_status = 2;
                        break;
                    }
                    input_buf_size *= 2;
                }
                input_buf[input_buf_pos] = nextchar;
                input_buf_pos ++;
            }
        }
    }
    //close the contexts
    mraa_gpio_close(mraa_gpio_ptr);
    mraa_aio_close(mraa_aio_ptr);
    // Close log file, if open
    if(log_fd != -1){
        close(log_fd);
    }
    return exit_status;
}