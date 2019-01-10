//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
  
static int cause_segfault;
static int catch;
static int dump_core;

// by assigning a value to the nullpointer (0), this function
// causes a segfault
void seg_subroutine(){
  char* b = 0;
  *b = 10;
}

int main(int argc, char** argv){
  int c;
  int option_index=0;
  static struct option long_options[] ={
    {"input", optional_argument, 0, 'i'},
    {"output", optional_argument, 0, 'o'},
    {"segfault", no_argument, &cause_segfault, 1},  //sets a flag
    {"catch", no_argument, &catch, 1},              //sets flag to catch seg faults
    {"dump-core", no_argument, &dump_core, 1},      //sets flag to dump core on seg-fault
    {0,0,0,0}
  };
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
        printf ("option -i with value '%s'\n", optarg);
        break;
      case 'o':
        printf ("option -o with value '%s'\n", optarg);
        break;
      default:
        fprintf(stderr, "Usage: ./lab0 [--input=filename] [--output=filename] [--segfault] [--catch] [--dump-core]\n");
        exit(3);
        break;

    }
  }
  if(cause_segfault){
    seg_subroutine();
    exit(2);
  }
  

  return 0;
}
