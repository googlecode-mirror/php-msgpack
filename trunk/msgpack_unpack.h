
#ifndef MSGPACL_UNPACK_H
#define MSGPACL_UNPACK_H

#include "msgpack/unpack_define.h"

typedef enum
{
    MSGPACK_UNPACK_SUCCESS     =  2,
    MSGPACK_UNPACK_EXTRA_BYTES =  1,
    MSGPACK_UNPACK_CONTINUE    =  0,
    MSGPACK_UNPACK_PARSE_ERROR = -1,
} msgpack_unpack_return;

#define VAR_ENTRIES_MAX 1024

typedef struct {
    zval *data[VAR_ENTRIES_MAX];
    long used_slots;
    void *next;
} var_entries;

#endif
