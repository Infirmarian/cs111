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

#define USED_CLOCK CLOCK_MONOTONIC
#define true 1
#define false 0
typedef int bool;

// define several locks to be used in a bitmask
#define MUTEX_LOCK 0x1
#define SPIN_LOCK 0x2
#define SWAP_LOCK 0x4

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct arg_struct {
		long long* pointer;
		long long value;
		int iterations;
		int lock;
		bool yield;
		volatile int spin_lock_value;
	};

// this function was taken from https://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
// and modified to support timespec rather than timeval
int timespec_subtract (struct timespec *result, struct timespec *x, struct timespec *y){
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int psec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
    y->tv_nsec -= 1000000000 * psec;
    y->tv_sec += psec;
  }
  if (x->tv_nsec - y->tv_nsec > 1000000000) {
    int psec = (x->tv_nsec - y->tv_nsec) / 1000000000;
    y->tv_nsec += 1000000000 * psec;
    y->tv_sec -= psec;
  }
  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void add(long long *pointer, long long value, bool yield, int lock_type, volatile int* spin_lock) {
	if(lock_type & MUTEX_LOCK){
		pthread_mutex_lock(&mutex);
		long long sum = *pointer + value;
		// yield if specified
		if (yield)
			sched_yield();
		*pointer = sum;
		pthread_mutex_unlock(&mutex);
		return;
	}
	if(lock_type & SPIN_LOCK){
	 
	  while(__sync_lock_test_and_set(spin_lock, 1)) // wait for the lock to be released
	    continue;

		long long sum = *pointer + value;
		if(yield)
			sched_yield();
		*pointer = sum;
		//printf("Done spinning, value is %lld\n", *pointer);
		__sync_lock_release(spin_lock); //release the lock

		return;
	}
	if(lock_type & SWAP_LOCK){
		long long new_sum;
		long long current;
		do{
		  current = *pointer;
		  new_sum = current + value;
		  if(yield)
			sched_yield();
			
		}while(__sync_val_compare_and_swap(pointer, current, new_sum) != current);
		//printf("Done with compare, value is %lld\n", *pointer);
		return;
	}
	// default, no synchronization
	long long sum = *pointer + value;
	// yield if specified
	if (yield)
		sched_yield();
	*pointer = sum;
	return;
}

void* threaded_add(void* args){
	struct arg_struct* a = (struct arg_struct*)args;
	for(int i = 0; i<a->iterations; i++){
	  add(a->pointer, a->value, a->yield, a->lock, &(a->spin_lock_value));
	}
	for(int i = 0; i<a->iterations; i++){
	  add(a->pointer, -1*a->value, a->yield, a->lock, &(a->spin_lock_value));
	}
	return 0;
}


int main(int argc, char** argv){
	int c;
	int option_index=0;
	long long counter = 0; // initialize the long long to be modified by each thread
	int thread_count = 1;
	int iterations = 1;
	int exit_status = 0;
	bool opt_yield = 0;
	char program_name[16] = "add";
	// struct for long options
	static struct option long_options[] ={
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", no_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0,0,0,0}
	};
	char lock = 0;
	// iterate through each argument and assign the values if needed
  	while(1){
    	c = getopt_long(argc, argv, "", long_options, &option_index);
      	if (c == -1){
       		break; 
		  }
		switch (c){
			case 't':
				thread_count = atoi(optarg);
				break;
			case 'i':
				iterations = atoi(optarg);
				break;
			case 'y':
				opt_yield = true;
				strcpy(program_name, "add-yield");
				break;
			case 's':
				switch(optarg[0]){
					case 'm': // mutex lock
					lock = 'm';
					break;
					case 's': // spin lock
					lock = 's';
					break;
					case 'c': // compare and swap
					lock = 'c';
					break;
					default:
						fprintf(stderr, "Bad option passed in for sync type\n");
						exit_status = 1;
				}
			break;
			case '?':
				exit_status = 1;
				break;
			default:
				exit(1);
		}
  	}
	  // set up the program name
	if(!lock){
		strcat(program_name, "-none");
	}else{
		int len = strlen(program_name);
		program_name[len] = '-';
		program_name[len+1] = lock;
		program_name[len+2] = '\0';
	}
	int lock_code = 0;
	switch(lock){
		case 'm':
			lock_code = MUTEX_LOCK;
			break;
		case 's':
			lock_code = SPIN_LOCK;
			break;
		case 'c':
			lock_code = SWAP_LOCK;
			break;
		default:
		break;
	}
	  // allocate [thread] number
	pthread_t* threads;
	threads = malloc(thread_count*sizeof(pthread_t));
	if(!threads){
		exit_status = 1;
		fprintf(stderr, "Error, unable to allocate memory to hold threads: %s", strerror(errno));
		if(fflush(stderr))
			fprintf(stderr, "Unable to flush stderr: %s", strerror(errno));
	}
	struct arg_struct a = {
		.iterations = iterations,
		.pointer = &counter,
		.value = 1,
		.lock = lock_code,
		.yield = opt_yield,
		.spin_lock_value = 0
	};
	// get starting time
	struct timespec init_time,final_time;
	if(clock_gettime(USED_CLOCK,&init_time)){
		fprintf(stderr, "Unable to get inital clock time: %s", strerror(errno));
		exit_status = 1;
	}
	// spawn threads
	for(int i = 0; i<thread_count; i++){
		if(pthread_create(&(threads[i]), 0, threaded_add, (void*)&a)){
			exit_status = 1;
			fprintf(stderr, "Unable to create a thread: %s", strerror(errno));
			fflush(stderr);
		}
	}
	// join threads
	for(int i = 0; i<thread_count; i++){
		if(pthread_join(threads[i], 0)){
			exit_status = 1;
			fprintf(stderr, "Unable to join a thread\n");
			fflush(stderr);
		}
	}
	//get clock ending time
	if(clock_gettime(USED_CLOCK, &final_time)){
		fprintf(stderr, "Unable to get final clock time: %s", strerror(errno));
		exit_status = 1;
	}

	long long op_count = iterations*thread_count*2;
	struct timespec diff;
	timespec_subtract(&diff, &final_time, &init_time);
	long long nsec_elapsed = diff.tv_sec*1000000000 + diff.tv_nsec;
	fprintf(stdout, "%s,%d,%d,%lld,%lld,%lld,%lld\n", program_name, thread_count, iterations, op_count, nsec_elapsed, nsec_elapsed/op_count, counter);
	free(threads);
	return exit_status;
}
