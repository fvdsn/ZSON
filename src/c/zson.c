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

int minsize32[] = {
    1,1,1,1,
    2,3,5,9, 2,3,5,9, 5,9,
    6,4,8,12,
    5,5,
    5,5,5,5, 5,5,5,5, 5,5
};

int minsize64[] = {
    1,1,1,1,
    2,3,5,9, 2,3,5,9, 5,9,
    10,4,8,12,
    9,9,
    9,9,9,9, 9,9,9,9, 9,9
};

int headersize32[] = {
    1,1,1,1,
    1,1,1,1, 1,1,1,1, 1,1,
    5,1,1,1,
    5,5,
    5,5,5,5, 5,5,5,5, 5,5
};

int headersize64[] = {
    1,1,1,1,
    1,1,1,1, 1,1,1,1, 1,1,
    9,1,1,1,
    9,9,
    9,9,9,9, 9,9,9,9, 9,9
};

#define GET_VAL(src,offset,type)  (((type*)(src + (offset)))[0])
zsnode_t* decode(const char *src, size_t maxsize, int bit64){
    int header  = src[0];
    size_t size = 0;
    if(header == 0 || header >= ZSON_TYPE_COUNT){
        return NULL; //TODO put error in context
    }

    int minsize = bit64 ? minsize64[header] : minsize32[header];
    int headersize = bit64 ? headersize64[header] : headersize32[header];
    if(minsize > maxsize){
        return NULL; //TODO put error in context
    }

    zsnode_t *z = (zsnode_t*)malloc(sizeof(zsnode_t));
    memset(z,0,sizeof(zsnode_t));
    z->type = header;
    z->entity = src;
    z->size = minsize;
    switch(header){
        case ZSON_NULL:
            break;
        case ZSON_TRUE:
            z->value.boolean = 1;
            break;
        case ZSON_FALSE:
            z->value.boolean = 0;
            break;
        case ZSON_INT8:
            z->value.int8 = (int8_t)src[1];
            break;
        case ZSON_INT16:
            z->value.int16 = GET_VAL(src,1,int16_t);
            break;
        case ZSON_INT32:
            z->value.int32 = GET_VAL(src,1,int32_t);
            break;
        case ZSON_INT64:
            z->value.int64 = GET_VAL(src,1,int64_t);
            break;
        case ZSON_UINT8:
            z->value.uint8 = (int8_t)src[1];
            break;
        case ZSON_UINT16:
            z->value.uint16 = GET_VAL(src,1,uint16_t);
            break;
        case ZSON_UINT32:
            z->value.uint32 = GET_VAL(src,1,uint32_t);
            break;
        case ZSON_UINT64:
            z->value.uint64 = GET_VAL(src,1,uint64_t);
            break;
        case ZSON_FLOAT32:
            z->value.float32 = GET_VAL(src,1,float);
            break;
        case ZSON_FLOAT64:
            z->value.float64 = GET_VAL(src,1,double);
            break;
        case ZSON_STRING4:
        case ZSON_STRING8:
        case ZSON_STRING12:
            z->value.string  = src + 1;
            if(src[z->size -1] != 0){
                return NULL; // string must be null terminated
            }
            z->length = strlen(z->value.string);
            break;
        default:
            if(bit64){
                size = GET_VAL(src,1,uint64_t);
            }else{
                size = GET_VAL(src,1,uint32_t);
            }
            if(size > maxsize || size < minsize){
                return NULL; // entity too long or too short !
            }
            z->size = size;
            size_t innersize = size - headersize;
            switch(header){
                case ZSON_STRING:
                    if(innersize == 0 || src[z->size -1] != 0){
                        return NULL; // zero length string must also be string terminated
                    }
                    z->value.string = src + headersize;
                    z->length = innersize - 1;
                    break;
                case ZSON_OBJECT:
                {
                    size_t offset  = headersize;
                    size_t length  = 0;
                    zsnode_t *last = NULL;
                    zsnode_t *el   = NULL;
                    zsnode_t *key  = NULL;
                    
                    while(offset < size){
                        key = decode(src + offset, maxsize - offset, bit64);
                        if(key->type < ZSON_STRING || key->type > ZSON_STRING12){
                            return NULL; // key must be a string
                        }
                        offset += key->size;
                        el = decode(src + offset, maxsize - offset, bit64);
                        if(!el){
                            return NULL; // error propagation
                        }else if(!last){
                            z->value.child = el;
                        }else{
                            last->next = el;
                        }
                        last = el;
                        el->key = key->value.string;
                        length++;
                        offset += el->size;
                        free(key);
                    }
                    
                    z->length = length;
                    break;
                }
                case ZSON_ARRAY:
                {
                    size_t offset  = headersize;
                    size_t length  = 0;
                    zsnode_t *last = NULL;
                    zsnode_t *el   = NULL;

                    while(offset < size){
                        el = decode(src + offset, maxsize - offset, bit64);
                        if(!el){
                            return NULL; //error propagation
                        }else if(!last){ //first element
                            z->value.child = el;
                        }else{
                            last->next = el;
                        }
                        last = el;
                        length++;
                        offset += el->size;
                    }

                    z->length = length;
                    break;
                }
                default:
                    return NULL;
            }
            break;
    }
    return z;
}

const char* TYPENAME[256] = {
    "ZSON_PADDING",
    "ZSON_NULL",
    "ZSON_TRUE",
    "ZSON_FALSE",
    "ZSON_INT8",
    "ZSON_INT16",
    "ZSON_INT32",
    "ZSON_INT64",
    "ZSON_UINT8",   
    "ZSON_UINT16",
    "ZSON_UINT32",
    "ZSON_UINT64",
    "ZSON_FLOAT32",
    "ZSON_FLOAT64",
    "ZSON_STRING",
    "ZSON_STRING4",
    "ZSON_STRING8",
    "ZSON_STRING12",
    "ZSON_OBJECT",
    "ZSON_ARRAY",
    "ZSON_ARRAY_INT8",
    "ZSON_ARRAY_INT16",
    "ZSON_ARRAY_INT32",
    "ZSON_ARRAY_INT64",
    "ZSON_ARRAY_UINT8",
    "ZSON_ARRAY_UINT16",
    "ZSON_ARRAY_UINT32",
    "ZSON_ARRAY_UINT64",
    "ZSON_ARRAY_FLOAT32",
    "ZSON_ARRAY_FLOAT64",
    "ZSON_TYPE_UNKNOWN"
};

void zsnode_fprintf(FILE *out, zsnode_t *z){
    if(z){
        fprintf(out,"ZSON NODE:\n");
        fprintf(out,"\ttype: %d, %s\n",z->type,TYPENAME[z->type < ZSON_TYPE_COUNT ? z->type: ZSON_TYPE_COUNT]);
        fprintf(out,"\tvalue: ");
        switch(z->type){
            case ZSON_PADDING: 
            case ZSON_NULL:     break;
            case ZSON_TRUE:
            case ZSON_FALSE:    fprintf(out,"%d",z->value.boolean); break;
            case ZSON_INT8:     fprintf(out,"%d",z->value.int8);    break;
            case ZSON_INT16:    fprintf(out,"%d",z->value.int16);   break;
            case ZSON_INT32:    fprintf(out,"%d",z->value.int32);   break;
            case ZSON_INT64:    fprintf(out,"%ld",z->value.int64);  break;
            case ZSON_UINT8:    fprintf(out,"%u",z->value.uint8);   break;
            case ZSON_UINT16:   fprintf(out,"%u",z->value.uint16);  break;
            case ZSON_UINT32:   fprintf(out,"%u",z->value.uint32);  break;
            case ZSON_UINT64:   fprintf(out,"%lu",z->value.uint64); break;
            case ZSON_FLOAT32:  fprintf(out,"%f",z->value.float32); break;
            case ZSON_FLOAT64:  fprintf(out,"%f",z->value.float64); break;
            case ZSON_STRING: 
            case ZSON_STRING4:
            case ZSON_STRING8:
            case ZSON_STRING12: fprintf(out,"%s",z->value.string); break;
            case ZSON_OBJECT:
            case ZSON_ARRAY:    fprintf(out,"childptr: %p",z->value.child); break;
        }
        fprintf(out,"\n");
        fprintf(out,"\tnext: %p\n",z->next);
        fprintf(out,"\tentity: %p\n",z->entity);
        fprintf(out,"\tkey: %s\n",z->key);
        fprintf(out,"\tsize: %d\n",z->size);
        fprintf(out,"\tlength: %d\n",z->length);
    }else{
        fprintf(out,"ZSON NODE: NULL\n");
    }
}
void zsnode_fprintf_rec(FILE* out, zsnode_t* z){
    zsnode_fprintf(out,z);
    if(z->type == ZSON_ARRAY || z->type == ZSON_OBJECT){
        fprintf(out,"<<<\n");
        zsnode_t *c = z->value.child;
        while(c){
            zsnode_fprintf_rec(out,c);
            c = c->next;
        }
        fprintf(out,">>>\n");
    }
}

int main(int argc, char** argv){
    //int mode      = DECODE;
    FILE *input_f = NULL;
    const char *input = NULL;
    size_t inputlen = 0;

    if(argc != 3 && argc != 4){
        goto argument_error;
    }

    if(!strcmp(argv[1],"encode")){
        //mode = ENCODE;
    }else if(!strcmp(argv[1],"decode")){
        //mode = DECODE;
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
    zsnode_t *z = decode(input,inputlen,0);
    zsnode_fprintf_rec(stdout,z);
    
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





