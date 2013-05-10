# ZSON

ZSON is a binary object serialisation format:

 - That has a schema as close as possible to JSON's.
 - That is very easy to understand and implement
 - That efficiently encodes arrays of numerical data
 - That can be very fast to write and parse, especially for numerical data
   (up to 10x write speedup, 10000x parsing speedup!)
 - That is relatively compact (between 1.5x longer to 3x shorter depending
   on type of data)
 - That can be efficiently seeked into and partially parsed 

## ZSON FAQ

### Isn't JSON good enough ? 

If JSON is good enough for you, then by all means, use JSON. If on the other hand
you need more performance but want to keep working on a simple and familiar encoding
scheme you could benefit from ZSON.

### Is ZSON faster to write and parse ?

All numbers, strings, and typed arrays are stored in ZSON as they would be stored natively 
in memory. All that's needed to parse those values is a mmapped pointer. This is hard to beat. 

### Is ZSON more compact ?

In the short tests so far, ZSON can be from 20% larger to 60% shorter than
JSON depending on the encoded data

### What are the failings of ZSON ?

 - Encoding large amount of small objects will take more space, and may be
   slower
 - There are several valid ways to encode a single schema
 - As the writer must backtrack, it is not possible to generate ZSON directly
   to a pipe or network.

### What applications is ZSON intended to ?

document containing binary blobs, multimedia objects, pictures, sound, 
numerical arrays, are likely to be more compact and a lot faster to write
and parse.



## ZSON Specification.

ZSON encodes the entities defined by the JSON standard:
  
  - null
  - true
  - false
  - numbers
  - strings
  - arrays
  - objects

In ZSON, numbers and arrays entities can carry type information,
the following type are supported: 32 and 64 bit floating point numbers,
8, 16, 32 and 64 bits signed and unsigned numbers


The entities are byte encoded in four parts.
   
  - The first byte indicates the type of the entity, the type is a
    constant numerical value that is encoded as an unsigned 8bit integer. This is the
    only field that is present in all entities encoding.
  - The next bytes may contain the total size of the entity in bytes encoded as
    an unsigned 32bit integer.
  - The padding consists of zero to 7 bytes of zeroed data that can used to
    align the encoded data
  - The data that is encoded as a byte sequence

<!-- WTF MARKDOWN -->

    MEMORY (BYTES)
    [    0   |    1    |    2    |    3    |  ....
    ENTITY:
    [ TYPE   |  ... SIZE ...     |   ... PADDING ...  |  ...  DATA  ...  ]

### Null, True, False

Are only Encoded by their type field.
    
    ENTITY : TYPE
    -------+-----
    NULL   :  1
    TRUE   :  2
    FALSE  :  3

### Numbers

Can be encoded in their different typed native represenation. The data is not 
padded. The size can be computed from the type and is thus not encoded.

    ENTITY  : TYPE : SIZE : REPR
    --------+------+------+-----
    INT8    : 4    : 2    : [ 4 | X ]
    INT16   : 5    : 3    : [ 5 | X X ]
    INT32   : 6    : 5    : [ 6 | X X X X ]
    INT64   :      : 9    : [   | X X X X X X X X ]
    UINT8   : 7    : 2    : [ 7 | X ]
    UINT16  : 8    : 3    : [ 8 | X X ]
    UINT32  : 9    : 5    : [ 9 | X X X X ]
    UINT64  :      : 9    : [   | X X X X X X X X ]
    FLOAT32 : 10   : 5    : [10 | X X X X ]
    FLOAT64 : 11   : 9    : [11 | X X X X X X X X ]

The encoder is free to choose any encoding for numerical values as long as it does
not result in any loss of precision. The decoder should provide the value as a
double

### Strings

Strings are encoded in utf8 and null terminated. The utf8 data is not padded.

    ENTITY : TYPE : SIZE
    -------+------+--------
    STRING :  12  : ENCODED

    [ 12 | Size | UTF8 ENCODED STRING ... | '\0' ]
    
There are compact encoding form for string that are less than eleven characters long
These encoding are simple and reduce string encoding size by 20% on average data, which is
enough to warrant their inclusion.

    ENTITY   : TYPE : SIZE : REPR
    -------  +------+------+-----
    STRING   :  12  : ENC  | [12 | S S S S ... 0 ] 
    STRING4  :      : 4    | [   | C C 0 ] 
    STRING8  :      : 8    | [   | C C C C C 0 ] 
    STRING12 :      : 12   | [   | C C C C C C C C C 0 ] 

    SIZE COMP:
    SRC | STR | STRX | JSON 
    ----+-----+------+------
    1   | 7   | 4    | 3    
    2   | 8   | 4    | 4    
    3   | 9   | 8    | 5    
    4   | 10  | 8    | 6    
    5   | 11  | 8    | 7    
    6   | 12  | 8    | 8    
    7   | 13  | 12   | 9    
    8   | 14  | 12   | 10   
    9   | 15  | 12   | 11   
    10  | 16  | 12   | 12   
    ----+-----+------+------
    -20 | +30 | +11  | 75   
    -28%| +40%| +15% | +0%


the string is less than 8 character long it can be encoded in compact form


### Arrays

An array is an ordered list of entities. The encoded array data simply consists of
the concatenated ZSON encoding of the array's entities, by increasing index order. If the array
contains `undefined` values, they must be encoded as `NULL`. The array data is not padded

    ENTITY : TYPE : SIZE
    -------+------+--------
    ARRAY  :  13  : ENCODED

    [ 13 | Size | VAL[0] | VAL[1] | ... ]
    
### Objects
Objects are key values stores were keys are string and values are any kind of entity. 
The Object encoded data is a concatenated sequence of the key values pairs. The key is
always encoded as a ZSON string and the value can be any ZSON encoded entity. The
Object data is not padded. If two key of the same value are encoded in the same object,
the decoded object takes the value of the last declaration

    ENTITY : TYPE : SIZE
    -------+------+--------
    OBJECT :  14  : ENCODED

    [ 14 | Size | KEY1 | VAL[KEY1] | KEY2 | VAL[KEY2 | ... ]

### Typed Arrays
If all the values of an array can be encoded with the same ZSON number type, the array
can be encoded as a ZSON typed array. In a typed array, the array data is encoded as a 
single concatenated sequence of the typed binary representation of the numerical values.
zero valued padding is inserted before the data so that the first byte of the data has
an offset relative to the start of the ZSON document that is a multiple of the size of the
binary representation of the numerical type. 

    ENTITY        : TYPE : SIZE    : PADDING
    --------------+------+---------+-----
    ARRAY_INT8    : 15   : ENCODED : NONE
    ARRAY_INT16   : 16   : ENCODED : %2
    ARRAY_INT32   : 17   : ENCODED : %4
    ARRAY_UINT8   : 18   : ENCODED : NONE
    ARRAY_UINT16  : 19   : ENCODED : %2
    ARRAY_UINT32  : 20   : ENCODED : %4
    ARRAY_FLOAT32 : 21   : ENCODED : %4
    ARRAY_FLOAT64 : 22   : ENCODED : %8

### Endianness

All values should be encoded and decoded with their 
big-endian binary representation.

### Bigger data

By default, entity sizes are encoded as unsigned 32bit unsigned ints, which limit
the size of the document to 4Go. An alternative encoding uses a 64bit 
Unsigned int. this can be specified in the manifest.

### Manifest

A ZSON document may be prefixed by a 8 byte manifest. The manifest
starts with the four ascii values of `zson` or `ZSON`. In the
presence of an upper-case manifest, a 64bit size encoding is assumed.

the next three bytes are intended for the user to encode the type of
document encoded, and could be for example the three first characters 
of the file extension.

The last byte is reserved for later use.

