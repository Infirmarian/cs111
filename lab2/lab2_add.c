//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	*pointer = sum;
}


int main(int argc, char** argv){
	int c;
	int option_index=0;
	long long counter = 0; // initialize the long long to be modified by each thread
	int threads = 1;
	int iterations = 1;
  
	// struct for long options
	static struct option long_options[] ={
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{0,0,0,0}
	};
	// iterate through each argument and assign the values if needed
  	while(1){
    	c = getopt_long(argc, argv, "", long_options, &option_index);
      	if (c == -1){
       		break; 
		  }
		switch (c){
			case 't':
				printf("Number of threads: %s\n", optarg);
				threads = atoi(optarg);
				break;
			case 'i':
				printf("Number of iterations: %s\n", optarg);
				iterations = atoi(optarg);
				break;
			case '?':
				printf("Unrecognized input\n");
				exit(1);
			default:
				exit(1);
		}
  	}
}