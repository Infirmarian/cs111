//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
int main(int argc, char** argv){
    int exit_status = 0;
    static struct option long_options[] ={
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", no_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0,0,0,0}
	};

    int option_index =0;
    int c;
    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1)
            break;
        switch(c){
            case '?':
            exit_status = 1;
            break;
            default:
            exit(2); // something went horribly wrong
        }
    }
    return exit_status;
}