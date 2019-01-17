//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// by assigning a value to the nullpointer (0), this function
// causes a segfault
void seg_subroutine(){
  char* b = 0;
  *b = 10;
}
//handles SIGSEGV faults and exits with error code 4
void sigsegv_handler(int s){
  fprintf(stderr, "SIGSEGV with code '%d'\n", s);
  exit(4);
}
//returns an int describing the input of the program
//on error, a message is printed out and program is terminated
//with error code 1
void redir_input(char* name){
  // opens the file with the name passed in
  int openfile = open(name, O_RDONLY);
  if(openfile < 0){
    fprintf(stderr, "Error with provided --input file, '%s': %s\n", name, strerror(errno));
    exit(2);
  }
  // closes the stdin
  if(close(STDIN_FILENO)){
    fprintf(stderr, "Error closing previous input stream: %s\n", strerror(errno));
    exit(2);
  }
  // redirects output to the recently closed file descriptor
  if(dup(openfile)<0){
    fprintf(stderr, "Error duplicating the input stream: %s\n", strerror(errno));
    exit(2);
  }
  // closes the duplicated file descriptor, leaving the same number of open files as before the function was executed
  if(close(openfile)){
    fprintf(stderr, "Error closing previous input stream: %s\n", strerror(errno));
    exit(2);
  }
}

//returns an int descriping the output of the program.
//on error, a message is printed out and the program is terminated
//with error code 2
void redir_output(char* name){
  int outfile = creat(name, 0666);
  if(outfile < 0){
    fprintf(stderr, "Error with provided --output file, '%s': %s\n", name, strerror(errno));
	exit(3);
  }
  if(close(STDOUT_FILENO)){
	  fprintf(stderr, "Error closing the previous output stream: %s", strerror(errno));
	  exit(3);
  }
  if(dup(outfile)<0){
	  fprintf(stderr, "Error duplicating the new output stream: %s", strerror(errno));
	  exit(3);
  }
  if(close(outfile)){
	fprintf(stderr, "Error closing the previous output stream: %s", strerror(errno));
	exit(3);
  }

}

int main(int argc, char** argv){
  int c;
  int option_index=0;
  static struct option long_options[] ={
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"segfault", no_argument, 0, 's'},        //sets a flag
    {"catch", no_argument, 0, 'c'},          //sets flag to catch seg faults
    {"dump-core", no_argument, 0, 'd'},      //sets flag to dump core on seg-fault
    {0,0,0,0}
  };

// this loop goes through and finds bogus arguments, before attempting to execute anything
  while(1){
    c = getopt_long(argc, argv, "", long_options, &option_index);
	if (c == -1)
		break; // all options were valid if this point is reached
    if(c == '?'){
      fprintf(stderr, "Unrecognized flag '%s' detected\nUsage: ./lab0 [--input=filename] [--output=filename] [--segfault] [--catch] [--dump-core]\n", argv[optind-1]);
      exit(1);
    }
  }
  
  option_index = 0;
  optind = 0; //reset option index to go through the arguments again
  while(1){
    c = getopt_long(argc, argv, "", long_options, &option_index);
    //end of the argument list
    if(c == -1){
      break;
    }
    switch(c){
      case 'i':
        redir_input(optarg);
        break;
      case 'o':
        redir_output(optarg);
        break;
      case 's':
        seg_subroutine();	// causes a segmentation fault
        break;
      case 'd':
	  	signal(SIGSEGV, SIG_DFL);	// reset the signal handler to dump again
		break;
      case 'c':
        signal(SIGSEGV, sigsegv_handler);	// set the signal handling function
        break;
      default:
        fprintf(stderr, "Usage: ./lab0 [--input=filename] [--output=filename] [--segfault] [--catch] [--dump-core]\n");
        exit(1);
		break;
    }
  }

  char buf[1];
  while(read(STDIN_FILENO, buf, 1))
    write(STDOUT_FILENO, buf, 1);

  //close input and output files
  //TODO: Error checking here
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  return 0;
}
