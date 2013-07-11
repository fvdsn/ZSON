#ifndef __ZSON_H__
#define __ZSON_H__
#include <stdbool.h>

enum ZSON_entity_type{
    ZSON_PADDING = 0,
    ZSON_NULL,
    ZSON_TRUE,
    ZSON_FALSE,
    ZSON_INT8,
    ZSON_INT16,
    ZSON_INT32,
    ZSON_INT64,
    ZSON_UINT8,   
    ZSON_UINT16,
    ZSON_UINT32,
    ZSON_UINT64,
    ZSON_FLOAT32,
    ZSON_FLOAT64,
    ZSON_STRING,
    ZSON_STRING4,
    ZSON_STRING8,
    ZSON_STRING12,
    ZSON_OBJECT,
    ZSON_ARRAY,
    ZSON_ARRAY_INT8,
    ZSON_ARRAY_INT16,
    ZSON_ARRAY_INT32,
    ZSON_ARRAY_INT64,
    ZSON_ARRAY_UINT8,
    ZSON_ARRAY_UINT16,
    ZSON_ARRAY_UINT32,
    ZSON_ARRAY_UINT64,
    ZSON_ARRAY_FLOAT32,
    ZSON_ARRAY_FLOAT64,
    ZSON_TYPE_COUNT,
};

enum ZSON_errors{
    ZSON_SUCCESS = 0,
    ZSON_ERROR_NULL_ARG,
    ZSON_ERROR_FILE_READ,
    ZSON_ERROR_INVALID_HEADER,
    ZSON_ERROR_INVALID_SIZE,
    ZSON_ERROR_RUNAWAY_STRING,
    ZSON_ERROR_INVALID_PADDING,
    ZSON_ERROR_INVALID_KEY,
    ZSON_ERROR_EMPTY,
    ZSON_ERROR_COUNT
};

typedef struct zsnode_t{
    int type;
    union{
        const char *string;
        int      boolean;
        float    float32;
        double   float64;
        int8_t   int8;
        int16_t  int16;
        int32_t  int32;
        int64_t  int64;
        uint8_t  uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;
        int8_t   *aint8;
        int16_t  *aint16;
        int32_t  *aint32;
        int64_t  *aint64;
        uint8_t  *auint8;
        uint16_t *auint16;
        uint32_t *auint32;
        uint64_t *auint64;
        const void* ptr;
    } value;
    const char *key;
    const void *content;
    const void *entity;
    size_t start;
    size_t length;
    size_t size;
    size_t content_start;
    bool has_child;
    bool parsed;
    bool has_next;
}zsnode_t;

typedef struct zson_t{
    FILE *file;
    const char *path;
    const char *mem;
    size_t mem_size;
    zsnode_t *stack;
    size_t stack_size;
    size_t stack_allocated_size;
    size_t stack_index;
    bool bit64;
    bool private_file;
    int error;
    const char *error_message;
} zson_t;

zson_t* zson_decode_path(const char *path);
zson_t* zson_decode_file(FILE *f);
zson_t* zson_decode_memory(const void *mem, size_t len);
void zson_free(zson_t *z);

bool zson_next(zson_t *z);
bool zson_child(zson_t *z);
bool zson_first_child(zson_t *z);
bool zson_parent(zson_t *z);
void zson_reset(zson_t *z);
bool zson_iterate(zson_t *z);
bool zson_has_child(zson_t *z);
bool zson_has_next(zson_t *z);
bool zson_has_parent(zson_t *z);
bool zson_can_iterate(zson_t *z);
bool zson_has_error(zson_t *z);
void zson_to_json(FILE *f, zson_t *z);

int  zson_get_type(zson_t *z);
bool zson_is_null(zson_t *z);
bool zson_is_number(zson_t *z);
bool zson_is_bool(zson_t *z);
bool zson_is_string(zson_t *z);
bool zson_is_array(zson_t *z);
bool zson_is_object(zson_t *z);
bool zson_is_typedarray(zson_t *z);

bool        zson_get_bool(zson_t *z);
double      zson_get_number(zson_t *z);
const char* zson_get_string(zson_t *z);
const char* zson_get_key(zson_t *z);
const void* zson_get_content_ptr(zson_t *z);

#endif
