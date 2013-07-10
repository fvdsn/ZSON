#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include "zson.h"

#define ENCODE 0
#define DECODE 1

static inline zsnode_t *zson_get_node(zson_t *z){
    return z->stack + z->stack_index;
}
static inline void zson_set_error(zson_t *z, int error, const char *msg){
    z->error = error;
    z->error_message = msg;
}
static void zson_push_node(zson_t *z){
    z->stack_index++;
    if(z->stack_index >= z->stack_size){
        z->stack_size = (size_t)(z->stack_index * 1.5);
        z->stack = realloc(z->stack, z->stack_size * sizeof(zsnode_t));
    }
    memset(z->stack + z->stack_index,0,sizeof(zsnode_t));
}
static void zson_pop_node(zson_t *z){
    if(z->stack_index > 0){
        z->stack_index--;
    }
}
static zson_t *zson_new(){
    zson_t *z = malloc(sizeof(zson_t));
    memset(z,0,sizeof(zson_t));
    z->stack_size = 4;
    z->stack = malloc(z->stack_size * sizeof(zsnode_t));
    memset(z->stack,0,z->stack_size * sizeof(zsnode_t));
    return z;
}

static int mapfile(FILE *f, const char **fptr, size_t *fsize){
    fseek(f,0,SEEK_END);
    size_t length = ftell(f);
    rewind(f);
    const char *input = mmap(NULL, length, 
        PROT_READ, MAP_PRIVATE, fileno(f),0);
    *fptr = input;
    *fsize = length;
    return 0;
}

zson_t *zson_decode_path(const char *path){
    zson_t *z = zson_new();
    if(!path){
        zson_set_error(z, ZSON_ERROR_NULL_ARG, "cannot read NULL path");
        return z;
    }
    FILE *f = fopen(path,"r");

    if(!f){
        zson_set_error(z, ZSON_ERROR_FILE_READ, "could not read input file");
        return z;
    }
    z->path = path;
    z->file = f;
    mapfile(f, &(z->mem), &(z->mem_size));
    return z;
}
zson_t *zson_decode_file(FILE *f){
    zson_t *z = zson_new();
    // *f; crash if file is NULL ?
    if(!f){
        zson_set_error(z, ZSON_ERROR_NULL_ARG, "cannot decode NULL file");
        return z;
    }
    z->path = NULL;
    z->file = f;
    mapfile(f,&(z->mem),&(z->mem_size));
    return z;
}
zson_t *zson_decode_memory(const void *mem, size_t len){
    zson_t *z = zson_new();
    if(!mem){
        zson_set_error(z, ZSON_ERROR_NULL_ARG, "cannot decode NULL");
        return z;
    }
    z->path = NULL;
    z->file = NULL;
    z->mem = mem;
    z->mem_size = len;
    return z;
}

int zson_get_type(zson_t *z){
    return zson_get_node(z)->type;
}
bool zson_is_number(zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    return zn->type >= ZSON_INT8 && zn->type <= ZSON_FLOAT64;
}
bool zson_is_string(zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    return zn->type >= ZSON_STRING && zn->type <= ZSON_STRING12;
}
bool zsnon_is_null(zson_t *z){
    return zson_get_node(z)->type == ZSON_NULL;
}
bool zson_is_bool(zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    return zn->type == ZSON_TRUE || zn->type == ZSON_FALSE;
}
bool zson_is_array(zson_t *z){
    return zson_get_node(z)->type == ZSON_ARRAY;
}
bool zson_is_object(zson_t *z){
    return zson_get_node(z)->type == ZSON_OBJECT;
}
bool zson_is_typedarray(zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    return zn->type >= ZSON_ARRAY_INT8 || zn->type <= ZSON_ARRAY_FLOAT64;
}
bool zson_get_bool(zson_t *z){
    return zson_get_node(z)->type == ZSON_TRUE;
}
double zson_get_number(zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    switch(zn->type){
        case ZSON_INT8:   return (double)zn->value.int8;
        case ZSON_INT16:  return (double)zn->value.int16;
        case ZSON_INT32:  return (double)zn->value.int32;
        case ZSON_INT64:  return (double)zn->value.int64;
        case ZSON_UINT8:  return (double)zn->value.uint8;
        case ZSON_UINT16: return (double)zn->value.uint16;
        case ZSON_UINT32: return (double)zn->value.uint32;
        case ZSON_UINT64: return (double)zn->value.uint64;
        case ZSON_FLOAT32: return (double)zn->value.float32;
        case ZSON_FLOAT64: return (double)zn->value.float64;
    }
    return 0.0;
}
const char* zson_get_string(zson_t *z){
    if(zson_is_string(z)){
        return zson_get_node(z)->value.string;
    }
    return NULL;
}
const char* zson_get_key(zson_t *z){
    return zson_get_node(z)->key;
}
const void* zson_get_content_ptr(zson_t *z){
    return zson_get_node(z)->content;
}
void zson_free(zson_t *z){
    free(z->stack);
    free(z);
    //TODO close files etc.
}

static uint8_t minsize32[] = {
    1,1,1,1,
    2,3,5,9, 2,3,5,9, 5,9,
    6,4,8,12,
    5,5,
    5,5,5,5, 5,5,5,5, 5,5
};

static uint8_t minsize64[] = {
    1,1,1,1,
    2,3,5,9, 2,3,5,9, 5,9,
    10,4,8,12,
    9,9,
    9,9,9,9, 9,9,9,9, 9,9
};

static uint8_t headersize32[] = {
    1,1,1,1,
    1,1,1,1, 1,1,1,1, 1,1,
    5,1,1,1,
    5,5,
    5,5,5,5, 5,5,5,5, 5,5
};

static uint8_t headersize64[] = {
    1,1,1,1,
    1,1,1,1, 1,1,1,1, 1,1,
    9,1,1,1,
    9,9,
    9,9,9,9, 9,9,9,9, 9,9
};

static uint8_t subsize[] = {
    0,0,0,0,
    0,0,0,0, 0,0,0,0, 0,0,
    0,0,0,0,
    0,0,
    1,2,4,8, 1,2,4,8, 4,8
};

#define GET_VAL(src,offset,type)  (((type*)(src + (offset)))[0])
static bool _decode( zson_t *zd, zsnode_t *z, const char *src, size_t start, 
                   size_t maxsize, bool keyval, bool bit64 ){
    memset(z,0,sizeof(zsnode_t));
    const char* origin = src;
    src = src+start;
    int header  = src[0];
    size_t size = 0;

    z->type = header;
    z->start = start;
    z->entity = src;
    z->parsed = true;
    z->has_next = false;

    if(header == 0 || header >= ZSON_TYPE_COUNT){
        zson_set_error(zd, ZSON_ERROR_INVALID_HEADER, "invalid header");
        return false;
    }

    int minsize = bit64 ? minsize64[header] : minsize32[header];
    int headersize = bit64 ? headersize64[header] : headersize32[header];
    z->content_start = headersize; 

    if(minsize > maxsize){
        zson_set_error(zd, ZSON_ERROR_INVALID_SIZE, "exceeding parent size");
        return false; 
    }

    z->content = src + headersize;
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
                zson_set_error( zd, ZSON_ERROR_RUNAWAY_STRING,
                               "strings must be null terminated");
                return false; 
            }
            break;
        default:
        {
            if(bit64){
                size = GET_VAL(src,1,uint64_t);
            }else{
                size = GET_VAL(src,1,uint32_t);
            }

            if(size > maxsize){
                zson_set_error( zd, ZSON_ERROR_INVALID_SIZE,
                                "size exceeds parent size");
                return false;
            }else if( size < minsize){
                zson_set_error(zd, ZSON_ERROR_INVALID_SIZE, "size too small");
                return false;
            }
            z->size = size;

            if (header == ZSON_STRING){
                if(size - headersize <= 0 || src[z->size -1] != 0){
                    zson_set_error( zd, ZSON_ERROR_RUNAWAY_STRING,
                                    "zero length strings must also be null terminated");
                    return false;
                }
                z->value.string = src + headersize;
                z->length = size - headersize - 1;
            }else if(header == ZSON_OBJECT || header == ZSON_ARRAY){
                z->has_child = size > headersize;
            }else if(header >= ZSON_ARRAY_INT8 && header <= ZSON_ARRAY_FLOAT64){
                if( size == headersize){
                    z->value.ptr = NULL;
                    z->length = 0;
                }else{
                    int padding = subsize[z->type];
                        padding = padding <= 1 ? 0 : (padding - start % padding) % padding;
                    if( headersize + padding > size){
                        zson_set_error( zd, ZSON_ERROR_INVALID_PADDING, 
                                        "padding going outside of entity");
                        return false;
                    }
                    z->value.ptr = src + headersize + padding;
                    z->length = (z->size - headersize - padding) / subsize[z->type];
                    z->content_start += padding;
                }
            }
        }
    }

    if(keyval){
        if(z->type < ZSON_STRING || z->type > ZSON_STRING12){
            zson_set_error( zd, ZSON_ERROR_INVALID_KEY, 
                            "key value pairs must start with a string" );
            return false;
        }
        if(!_decode(zd, z, origin, start+size, 
                           maxsize-size, false, bit64 )){
            return false;
        }
        z->key = src + headersize;
        z->size += size;
        z->content_start += size;
        z->start = start;
    }

    return z->type;
}

bool zson_next(zson_t *z){
    zsnode_t *cz = z->stack + z->stack_index;

    if(zson_has_error(z)){
        return false;
    }else if(z->stack_index == 0 && !cz->parsed){
        _decode(z, cz, z->mem, 0, z->mem_size, false, z->bit64);
        return !zson_has_error(z);
    }else if(cz->has_next){
        zsnode_t *pz = z->stack + z->stack_index - 1;
        size_t start = cz->start + cz->size;
        _decode(z, cz, z->mem, start, pz->size + pz->start - start, 
                       pz->type == ZSON_OBJECT, z->bit64 );
        cz->has_next = cz->start + cz->size < pz->start + pz->size;
        return !zson_has_error(z);
    }
    return false;
}
bool zson_child(zson_t *z){
    zsnode_t *pz = z->stack + z->stack_index;
    if(zson_has_error(z) || !pz->parsed){
        return false;
    }else if(pz->has_child){
        zson_push_node(z);
        zsnode_t *cz = z->stack + z->stack_index;
        _decode(z, cz, z->mem, pz->start + pz->content_start, 
                              pz->size - pz->content_start, 
                              pz->type == ZSON_OBJECT, z->bit64 ); 
        cz->has_next = cz->start + cz->size < pz->start + pz->size;
        return !zson_has_error(z);
    }

    return false;
}
bool zson_parent(zson_t *z){
    if( zson_has_error(z)){ 
        return false;
    }else if(z->stack_index > 0){
        zson_pop_node(z);
        return !zson_has_error(z);
    }
    return false;
}
bool zson_has_child(zson_t *z){
    return zson_get_node(z)->has_child;
}
bool zson_has_next(zson_t *z){
    return !zson_get_node(z)->parsed || zson_get_node(z)->has_next;
}
bool zson_has_parent(zson_t *z){
    return z->stack_index > 0;
}
bool zson_has_error(zson_t *z){
    return (bool)z->error; 
}
void zson_print_error(FILE *out, zson_t *z){
    if(zson_has_error(z)){
        fprintf( out,"ZSON_ERROR #%d AT OFFSET %d : %s\n",
                 z->error, zson_get_node(z)->start, z->error_message );
    }
}
static void zson_print_mem(FILE* out, zson_t *z){
    if(z){
        fprintf(out,"MEM[%d]: ",z->mem_size);
        for(int i = 0; i < z->mem_size; i++){
            int c = z->mem[i];
            if(c < 10){
                fprintf(out,"%d   ",c);
            }else if(c < 100){
                fprintf(out,"%d  ",c);
            }else{
                fprintf(out,"%d ",c);
            }
        }
        fprintf(out,"\n");
    }
}
const char* TYPENAME[256] = {
    "PADDING",
    "NULL",
    "TRUE",
    "FALSE",
    "INT8",
    "INT16",
    "INT32",
    "INT64",
    "UINT8",   
    "UINT16",
    "UINT32",
    "UINT64",
    "FLOAT32",
    "FLOAT64",
    "STRING",
    "STRING4",
    "STRING8",
    "STRING12",
    "OBJECT",
    "ARRAY",
    "ARRAY_INT8",
    "ARRAY_INT16",
    "ARRAY_INT32",
    "ARRAY_INT64",
    "ARRAY_UINT8",
    "ARRAY_UINT16",
    "ARRAY_UINT32",
    "ARRAY_UINT64",
    "ARRAY_FLOAT32",
    "ARRAY_FLOAT64",
    "TYPE_UNKNOWN"
};

void zson_print(FILE *out, zson_t *z){
    if(z){
        fprintf(out,"ZSON HANDLE:\n");
        fprintf(out,"\tpath: %s\n",z->path);
        fprintf(out,"\tmem: %p\n",z->mem);
        fprintf(out,"\tmem_size: %d\n",z->mem_size);
        fprintf(out,"\tstack: %p\n",z->stack);
        fprintf(out,"\tstack_size: %d\n",z->stack_size);
        fprintf(out,"\tstack_index: %d\n",z->stack_index);
        fprintf(out,"\tbit64: %d\n",(int)z->bit64);
    }else{
        fprintf(out,"ZSON HANDLE: NULL\n");
    }
}
void zson_print_value(FILE *out, zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    switch(zn->type){
        case ZSON_INT8:     fprintf(out,"%d",zn->value.int8);    break;
        case ZSON_INT16:    fprintf(out,"%d",zn->value.int16);   break;
        case ZSON_INT32:    fprintf(out,"%d",zn->value.int32);   break;
        case ZSON_INT64:    fprintf(out,"%ld",(long int)zn->value.int64);  break;
        case ZSON_UINT8:    fprintf(out,"%u",zn->value.uint8);   break;
        case ZSON_UINT16:   fprintf(out,"%u",zn->value.uint16);  break;
        case ZSON_UINT32:   fprintf(out,"%u",zn->value.uint32);  break;
        case ZSON_UINT64:   fprintf(out,"%lu",(long unsigned int)zn->value.uint64); break;
        case ZSON_FLOAT32:  fprintf(out,"%f",zn->value.float32); break;
        case ZSON_FLOAT64:  fprintf(out,"%f",zn->value.float64); break;
        case ZSON_STRING: 
        case ZSON_STRING4:
        case ZSON_STRING8:
        case ZSON_STRING12: fprintf(out,"%s",zn->value.string); break;
    }
}
void zson_print_node(FILE *out, zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    if(zson_get_key(z)){
        fprintf(out," %s : ",zson_get_key(z));
    }
    fprintf(out,"%s:", TYPENAME[zn->type < ZSON_TYPE_COUNT ? zn->type: ZSON_TYPE_COUNT]);
    zson_print_value(out,z);
    fprintf(out,"\n");
}
void zson_print_full_node(FILE *out, zson_t *z){
    zsnode_t *zn = zson_get_node(z);
    if(z){
        fprintf(out,"ZSON NODE:\n");
        fprintf(out,"\ttype: %d, %s\n",zn->type,TYPENAME[zn->type < ZSON_TYPE_COUNT ? zn->type: ZSON_TYPE_COUNT]);
        fprintf(out,"\tvalue: ");
        zson_print_value(out,z);
        fprintf(out,"\n");
        fprintf(out,"\tkey: %s\n",zn->key);
        fprintf(out,"\tcontent: %p\n",zn->content);
        fprintf(out,"\tentity: %p\n",zn->entity);
        fprintf(out,"\tstart: %d\n",zn->start);
        fprintf(out,"\tlength: %d\n",zn->length);
        fprintf(out,"\tsize: %d\n",zn->size);
        fprintf(out,"\tcontent_start: %d\n",zn->content_start);
        fprintf(out,"\thas_child: %d\n",(int)zn->has_child);
        fprintf(out,"\tparsed: %d\n",(int)zn->parsed);
        fprintf(out,"\thas_next: %d\n",(int)zn->has_next);
    }else{
        fprintf(out,"ZSON NODE: NULL\n");
    }
}

int main(int argc, char** argv){
    //int mode      = DECODE;

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

    zson_t *z = zson_decode_path(argv[2]); 
    
    while(zson_child(z) || zson_next(z) || (zson_parent(z) && zson_next(z))){
        zson_print_node(stdout,z);
    }
    if(zson_has_error(z)){
        zson_print_error(stdout,z);
        zson_print_mem(stdout,z);
    }

    zson_free(z);
    
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





