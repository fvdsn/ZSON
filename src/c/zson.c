#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include "zson.h"

#define ENCODE 0
#define DECODE 1

int mapfile(FILE *f, const char **fptr, size_t *fsize){
    fseek(f,0,SEEK_END);
    size_t length = ftell(f);
    rewind(f);
    const char *input = mmap(NULL, length, 
        PROT_READ, MAP_PRIVATE, fileno(f),0);
    *fptr = input;
    *fsize = length;
    return 0;
}

int main(int argc, char** argv){
    int mode      = DECODE;
    FILE *input_f = NULL;
    const char *input = NULL;
    size_t inputlen = 0;

    if(argc != 3 && argc != 4){
        goto argument_error;
    }

    if(!strcmp(argv[1],"encode")){
        mode = ENCODE;
    }else if(!strcmp(argv[1],"decode")){
        mode = DECODE;
    }else{
        goto argument_error;
    }

    input_f = fopen(argv[2],"r");
    if(!input_f){
        fprintf(stderr,"ERROR: could not read input file: %s\n",argv[2]); 
        return EXIT_FAILURE;
    }
    mapfile(input_f, &input, &inputlen);
    fprintf(stdout,"File size: %d\n",inputlen);
    int y = 0;
    for(int i=0; i < inputlen; i += 100000){
        y += input[i];
    }
    fprintf(stdout,"Result: %d\n",y);
    
    /*if(argc == 4){
        output_f = fopen(argv[3],"w");
        if(!output_f){
            fprintf(stderr,"ERROR: could not write to file: %s\n",argv[3]);
            return EXIT_FAILURE;
        }
    }else{
        output_f = stdout;
    }*/
    return EXIT_SUCCESS;

argument_error:
    fprintf(stderr,"USAGE: zson {encode|decode} INPUT_FILE [OUTPUT_FILE]\n");
    return EXIT_FAILURE;
}





