# ZSON

ZSON is a binary object serialisation format:

 - That has one to one correspondance with JSON; Any valid JSON
   has a valid ZSON encoding, and any ZSON encoding has a valid JSON
   encoding.
 - That is very easy to understand and implement
 - That efficiently encodes TypedArrays
 - That can be very fast to write and parse, especially for numerical data
   (up to 10x write speedup, 10000x parsing speedup!)
 - That is relatively compact (between 1.5x longer to 3x shorter depending
   on type of data)
 - That can be efficiently seeked into and partially parsed 
 - That supports big and little endianness

## ZSON Specification.

ZSON encodes the entities defined by the JSON standard.

The entities are byte encoded in four parts.
   
  - The first byte indicates the type of the entity, the type is a
    constant numerical value that is encoded as an unsigned 8bit integer. This is the
    only field that is always present.
  - The next bytes may contain the total size of the entity in bytes encoded as
    an unsigned 32bit integer.
  - The padding consists of zero to 7 bytes of zeroed data that can used to
    align the encoded data
  - The data that is encoded as a byte sequence

.

    MEMORY (BYTES)
    [    0   |    1    |    2    |    3    |  ....
    ENTITY:
    [ TYPE   |  ... SIZE ...     |   ... PADDING ...  |  ...  DATA  ...  ]

### Null, True, False

Are only Encoded by their type field.
    
    ENTITY : TYPE
    -------+-----
    TRUE   :  1
    FALSE  :  2
    NULL   :  3

### Numbers

Can be encoded in their different typed native represenation. The data is not 
padded. The size can be computed from the type and is thus not encoded.

    ENTITY  : TYPE : SIZE
    --------+------+-----
    INT8    : 4    : 2
    INT16   : 5    : 3
    INT32   : 6    : 5
    UINT8   : 7    : 2
    UINT16  : 8    : 3
    UINT32  : 9    : 5
    FLOAT32 : 10   : 5
    FLOAT64 : 11   : 9

The encoder is free to choose any encoding for numerical values as long as it does
not result in any loss of precision. The decoder should provide the value as a
double

### Strings

Strings are encoded in utf8 and null terminated. The utf8 data is not padded.

    ENTITY : TYPE : SIZE
    -------+------+--------
    STRING :  12  : ENCODED

    [ 12 | Size | UTF8 ENCODED STRING ... | '\0' ]

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

By default, all values should be encoded and decoded with their 
big-endian binary representation. A little-endian encoding may be
specifed in the `Manifest`

### Bigger data

By default, entity sizes are encoded as unsigned 32bit ints, which limit
the size of the document to 4Go. An alternative encoding uses a 64bit 
Floating Point for all the entities size. Only positive integer values are
considered valid, which limits the size of the document to 2 ^ 53 Bytes.
That should be enough for everybody. 

### Manifest

A ZSON document may be prefixed by a 8 byte manifest. The manifest
starts with the four ascii values of the `ZSON` 
string. If the next byte is equal to`'\0'`, The document is
encoded with 32bit  size. If the next byte is equal to `'\0'`,
The document is encoded with big endian representation. If any
of these two values are different from `'\0'`, the manifest is required. The next byte
is unused, and the last byte is an unsigned 8bit int used for version 
numbering. 

