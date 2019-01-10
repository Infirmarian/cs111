//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

//input flags
static int segfault_flag;
static int catch_flag;
static int dump_core_flag;

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

int main(int argc, char** argv){
  int c;
  int option_index=0;
  static struct option long_options[] ={
    {"input", optional_argument, 0, 'i'},
    {"output", optional_argument, 0, 'o'},
    {"segfault", no_argument, &segfault_flag, 1},  //sets a flag
    {"catch", no_argument, &catch_flag, 1},              //sets flag to catch seg faults
    {"dump-core", no_argument, &dump_core_flag, 1},      //sets flag to dump core on seg-fault
    {0,0,0,0}
  };
  char* inputs = 0;
  char* outputs = 0;
  while(1){
    c=getopt_long(argc, argv, "", long_options, &option_index);
    //end of the argument list
    if(c == -1){
      break;
    }

    switch(c){
      case 0:
        if(long_options[option_index].flag != 0)
          break;
      case 'i':
        inputs = optarg;
        break;
      case 'o':
        outputs=optarg;
        break;
      default:
        fprintf(stderr, "Usage: ./lab0 [--input=filename] [--output=filename] [--segfault] [--catch] [--dump-core]\n");
        exit(3);
        break;

    }
  }
  //sets up sigsegv_handler to catch segfaults
  if(catch_flag && !dump_core_flag){
    signal(SIGSEGV, sigsegv_handler);
  }
  //cause a segmentation fault
  if(segfault_flag){
    seg_subroutine();
    exit(2);
  }
  int instream;
  int outstream;
  // file opening
  if(inputs){
    instream = open(inputs, O_RDONLY);
  }else{
    instream = STDIN_FILENO;
    printf("Input file: STDIN\n");
  }
  if(outputs){
    outputs = open(outputs, O_WRONLY);
    printf("Output file: %s\n", outputs);
  }else{
    outstream = STDOUT_FILENO;
    printf("Output file: STDOUT\n");
  }
  //catch errors with 
  

  return 0;
}
