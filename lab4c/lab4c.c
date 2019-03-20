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
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include "mraa.h"


#define celsius 0
#define fahrenheit 1
#define POLL_INTERVAL 100


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
int Write(int fd, char* buf, int nbytes){
    int res = write(fd, buf, nbytes);
    if(res == -1){
        fprintf(stderr, "Unable to write bytes out to buffer: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        return 2;
    }
    return 0;
}
int setup_tcp(char* hostname, char* port);
int setup_tls(int fd, SSL** ssl);

static struct option long_options[] = {
    {"period", required_argument, 0, 'p'},
    {"scale", required_argument, 0, 's'},
    {"log", required_argument, 0, 'l'},
    {"id", required_argument, 0, 'i'},
    {"host", required_argument, 0, 'h'},
    {0,0,0,0}
};

int main(int argc, char** argv){
    int period = 1;
    int scale = 0;
    char* out_file_name = 0;
    char* hostname = 0;
    char* port = 0;
    int log_fd = -1;
    int exit_status = 0;
    int id = 0;
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
            case 'i':
                id = atol(optarg);
                if(id > 999999999 || id < 100000000){
                    fprintf(stderr, "Invalid ID entered\n");
                    exit(1);
                }
                break;
            case 'h':
                hostname = optarg;
                break;
            default:
                fprintf(stderr, "Unrecognized argument");
                exit(1);
                break;
        }
    }
    // Get the port number
    int index;
    for (index = optind; index < argc; index++){
        int x = atol(argv[index]);
        if(x > 0){
            port = argv[index];
            break;
        }
    }
    // Check that the required arguments were provided
    if(id == 0 || out_file_name == 0 || hostname == 0 || port==0){
        fprintf(stderr, "Failed to provide 1 or more required arguments\n");
        fflush(stderr);
        exit(1);

    }

    // Open the provided log file as output (if it exists)
    if(out_file_name){
        log_fd = open(out_file_name, O_WRONLY | O_APPEND | O_CREAT);
        if(log_fd == -1){
            fprintf(stderr, "Unable to open the stated log file: %s", strerror(errno));
            exit_status = 1;
        }
    }
    int socket_fd = 0;
    socket_fd = setup_tcp(hostname, port);
    //Initialize Networking
    #if defined(TLS)
    SSL* ssl = 0;
    exit_status = setup_tls(socket_fd, &ssl) ? 2 : exit_status;
    #endif 

    // Write to the server the ID=value
    char id_buffer[15];
    sprintf(id_buffer, "ID=%d\n", id);
    exit_status = Write(log_fd, id_buffer, strlen(id_buffer)) ? 2 : exit_status;
    #if defined(TLS)
    SSL_write(ssl, id_buffer, strlen(id_buffer));
    #else
    write(socket_fd, id_buffer, strlen(id_buffer));
    #endif

    //Initialize the GPIO switch
    mraa_aio_context mraa_aio_ptr = mraa_aio_init(1);
    if(mraa_aio_ptr == 0){
        fprintf(stderr, "Unable to initialize the Analog Input for the temperature sensor\n");
        fflush(stderr);
        exit_status = 2;
    }

    struct pollfd pfd;
    pfd.fd = socket_fd;
    pfd.events = POLLIN;
    time_t next_output_time = time(0);

    // Loop temperature and polling for input
    int raw_temp;
    char buf[30];
    char nextchar;
    char* input_buf = malloc(sizeof(char)* 200);
    if(input_buf == 0){
        fprintf(stderr, "Unable to allocate space for user commands: %s", strerror(errno));
        fflush(stderr);
    }
    int input_buf_size = 200;
    int input_buf_pos = 0;
    int reporting = 0;
    while(1){
        if(time(0) >= next_output_time && reporting){
            next_output_time = time(0) + period;
            //Produce report
            raw_temp = mraa_aio_read(mraa_aio_ptr);
            if(raw_temp == -1){
                fprintf(stderr, "Unable to read temperature\n");
                exit_status = 2;
            }else{
                next_output(buf, convert_temperature(raw_temp, scale));
                #if defined(TLS)
                exit_status = SSL_write(ssl, buf, strlen(buf)) <= 0 ? 2: exit_status;
                #else
                exit_status = Write(socket_fd, buf, strlen(buf)) ? 2 : exit_status;
                #endif
                if(log_fd != -1){
                    exit_status = Write(log_fd, buf, strlen(buf)) ? 2 : exit_status;
                }
            }
        }
        poll(&pfd, 1, POLL_INTERVAL);
        // read and deal with input, if available
        if(pfd.revents & POLLIN){
            #if defined(TLS)
            SSL_read(ssl, &nextchar, 1);
            #else
            read(socket_fd, &nextchar, 1);
            #endif
            if(nextchar == '\n'){
                //Process command
                int valid_command=0;
                if(strncmp("SCALE=F", input_buf, input_buf_pos) == 0){
                    scale = fahrenheit;
                    valid_command = 1;
                }
                if(strncmp("SCALE=C", input_buf, input_buf_pos) == 0){
                    scale = celsius;
                    valid_command = 1;
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
                    valid_command = 1;
                }
                if(strncmp("START", input_buf, input_buf_pos) == 0){
                    reporting = 1;
                    valid_command = 1;
                }
                if(strncmp("STOP", input_buf, input_buf_pos) == 0){
                    reporting = 0;
                    valid_command = 1;
                }
                if(strncmp("LOG ", input_buf, 4) == 0){
                    valid_command = 1;
                }
                if(strncmp("OFF", input_buf, input_buf_pos) == 0){
                    char sd_buf[30];
                    shutdown_message(sd_buf);
                    #if defined(TLS)
                        SSL_write(ssl, sd_buf, strlen(sd_buf));
                    #else
                        exit_status = Write(socket_fd, sd_buf, strlen(sd_buf)) ? 2 : exit_status;
                    #endif
                    if(log_fd != -1){
                        exit_status = Write(log_fd, input_buf, input_buf_pos) ? 2 : exit_status;
                        exit_status = Write(log_fd, "\n", 1) ? 2 : exit_status;
                        exit_status = Write(log_fd, sd_buf, strlen(sd_buf)) ? 2 : exit_status;
                    }
                    break;
                }
                // Log the command, if logging in enabled
                if(log_fd != -1){
                    exit_status = Write(log_fd, input_buf, input_buf_pos) ? 2 : exit_status;
                    exit_status = Write(log_fd, "\n", 1) ? 2 : exit_status;
                }
                if(! valid_command){
                    fprintf(stderr, "Invalid command received\n");
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
    mraa_aio_close(mraa_aio_ptr);
    // Close log file, if open
    if(log_fd != -1){
        close(log_fd);
    }
    close(socket_fd);
    #if defined(TLS)
        SSL_shutdown(ssl);
        SSL_free(ssl);
    #endif

    return exit_status;
}
int setup_tls(int fd, SSL** ssl){
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX* ctx = 0;
    SSL* data = 0;
    int error = 0;
    const SSL_METHOD* method = SSLv23_method();
    ctx = SSL_CTX_new(method);
    if(!ctx){
        fprintf(stderr, "Unable to create SSL_CTX Object\n");
        error = 2;
    }
    data = SSL_new(ctx);
    int x = SSL_set_fd(data, fd);
    if(x == 0){
        fprintf(stderr, "Unable to link the SSL structure to the socket fd\n");
        error = 2;
    }
    x = SSL_connect(data);
    if(x <= 0){
        fprintf(stderr, "Unable to connect to SSL\n");
        error = 2;
    }
    *ssl = data;
    return error;
}

int setup_tcp(char* hostname, char* port){
    // This code was sourced from https://manpages.debian.org/jessie/manpages-dev/getaddrinfo.3.en.html
    // and lightly modified
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;   
    hints.ai_socktype = SOCK_STREAM;     

    if (getaddrinfo(hostname, port, &hints, &result)) {
        fprintf(stderr, "Failed to get address info\n");
        exit(2);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */
    int socket_fd;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socket_fd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (socket_fd == -1)
            continue;

        if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
        close(socket_fd);
    }
    // Free the memory of the address information, as its not needed anymore
    freeaddrinfo(result);
    return socket_fd;
}

