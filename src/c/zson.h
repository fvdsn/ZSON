#ifndef __ZSON_H__
#define __ZSON_H__

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

typedef struct zsnode_t{
    int type;
    union{
        struct zsnode_t *child;
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
    } value;
    struct zsnode_t *next;
    const char *key;
    const void *entity;
    size_t size;
    size_t length;
}zsnode_t;

#endif
