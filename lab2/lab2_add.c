//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>


struct arg_struct {
		long long* pointer;
		long long value;
		int iterations;
	};

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	*pointer = sum;
}

void* threaded_add(void* args){
	struct arg_struct* a = (struct arg_struct*)args;
	for(int i = 0; i<a->iterations; i++){
		add(a->pointer, a->value);
	}
	for(int i = 0; i<a->iterations; i++){
		add(a->pointer, -1*a->value);
	}
	return 0;
}


int main(int argc, char** argv){
	int c;
	int option_index=0;
	long long counter = 0; // initialize the long long to be modified by each thread
	int thread_count = 1;
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
				thread_count = atoi(optarg);
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

	  // allocate [thread] number
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	if(!threads){
		fprintf(stderr, "Error, unable to allocate memory to hold threads: %s", strerror(errno));
		if(fflush(stderr))
			fprintf(stderr, "Unable to flush stderr: %s", strerror(errno));
	}
	struct arg_struct a;
		a.iterations = iterations;
		a.pointer = &counter;
		a.value = -1;
	
	for(int i = 0; i<thread_count; i++){
		pthread_create(threads+(i*sizeof(pthread_t)), 0, threaded_add, (void*)&a);
	}
	for(int i = 0; i<thread_count; i++){
		pthread_join(*(threads+i*sizeof(pthread_t)), 0);
	}
	fprintf(stdout, "The final result value of pointer is %lld\n", counter);

	free(threads);
}