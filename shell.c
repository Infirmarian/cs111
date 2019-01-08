//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>


int main(int argc, char** argv){
  printf("Hello World\n");
  printf("Hello again!\n");
  int c;
  int option_index=0;
  static struct option long_options[] ={
    {"input", optional_argument, 0, 'i'},
    {"output", optional_argument, 0, 'o'}
    {}
    {0,0,0,0}
  };
  while(1){
    c=getopt_long(argc, argv, "", long_options, &option_index);
    //end of the argument list
    if(c == -1){
      break;
    }
    switch(c){
      case 'i':
        printf ("option -i with value '%s'\n", optarg);
        break;
      default:
        abort();

    }
  }
  

  return 0;
}
