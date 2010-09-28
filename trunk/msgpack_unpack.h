
#ifndef MSGPACK_UNPACK_H
#define MSGPACK_UNPACK_H

#include "ext/standard/php_var.h"

#define MSGPACK_EMBED_STACK_SIZE 1024

#include "msgpack/unpack_define.h"

typedef enum
{
    MSGPACK_UNPACK_SUCCESS     =  2,
    MSGPACK_UNPACK_EXTRA_BYTES =  1,
    MSGPACK_UNPACK_CONTINUE    =  0,
    MSGPACK_UNPACK_PARSE_ERROR = -1,
} msgpack_unpack_return;

typedef struct {
    zval *retval;
    long deps;
    php_unserialize_data_t *var_hash;
    long stack[MSGPACK_EMBED_STACK_SIZE];
    int type;
} msgpack_unserialize_data;

void msgpack_unserialize_var_init(php_unserialize_data_t *var_hashx);
void msgpack_unserialize_var_destroy(php_unserialize_data_t *var_hashx);

void msgpack_unserialize_init(msgpack_unserialize_data *unpack);
int msgpack_unserialize_uint8(
    msgpack_unserialize_data *mp_unpack, uint8_t data, zval **obj);
int msgpack_unserialize_uint16(
    msgpack_unserialize_data *mp_unpack, uint16_t data, zval **obj);
int msgpack_unserialize_uint32(
    msgpack_unserialize_data *mp_unpack, uint32_t data, zval **obj);
int msgpack_unserialize_uint64(
    msgpack_unserialize_data *mp_unpack, uint64_t data, zval **obj);
int msgpack_unserialize_int8(
    msgpack_unserialize_data *mp_unpack, int8_t data, zval **obj);
int msgpack_unserialize_int16(
    msgpack_unserialize_data *mp_unpack, int16_t data, zval **obj);
int msgpack_unserialize_int32(
    msgpack_unserialize_data *mp_unpack, int32_t data, zval **obj);
int msgpack_unserialize_int64(
    msgpack_unserialize_data *mp_unpack, int64_t data, zval **obj);
int msgpack_unserialize_float(
    msgpack_unserialize_data *mp_unpack, float data, zval **obj);
int msgpack_unserialize_double(
    msgpack_unserialize_data *mp_unpack, double data, zval **obj);
int msgpack_unserialize_nil(msgpack_unserialize_data *mp_unpack, zval **obj);
int msgpack_unserialize_true(msgpack_unserialize_data *mp_unpack, zval **obj);
int msgpack_unserialize_false(msgpack_unserialize_data *mp_unpack, zval **obj);
int msgpack_unserialize_raw(
    msgpack_unserialize_data *mp_unpack, const char* base, const char* data,
    unsigned int len, zval **obj);
int msgpack_unserialize_array(
    msgpack_unserialize_data *mp_unpack, unsigned int count, zval **obj);
int msgpack_unserialize_array_item(
    msgpack_unserialize_data *mp_unpack, zval **container, zval *obj);
int msgpack_unserialize_map(
    msgpack_unserialize_data *mp_unpack, unsigned int count, zval **obj);
int msgpack_unserialize_map_item(
    msgpack_unserialize_data *mp_unpack, zval **container,
    zval *key, zval *val);


/* template functions */
#define msgpack_unpack_struct(name)    struct template ## name
#define msgpack_unpack_func(ret, name) ret template ## name
#define msgpack_unpack_callback(name)  template_callback ## name

#define msgpack_unpack_object zval*
#define unpack_user           msgpack_unserialize_data
#define msgpack_unpack_user   msgpack_unserialize_data

struct template_context;
typedef struct template_context msgpack_unpack_t;

static void template_init(msgpack_unpack_t* unpack);
static msgpack_unpack_object template_data(msgpack_unpack_t* unpack);
static int template_execute(
    msgpack_unpack_t* unpack, const char* data, size_t len, size_t* off);

static inline msgpack_unpack_object template_callback_root(unpack_user* user)
{
    msgpack_unserialize_init(user);
    return NULL;
}

static inline int template_callback_uint8(
    unpack_user* user, uint8_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_uint8(user, data, obj);
}

static inline int template_callback_uint16(
    unpack_user* user, uint16_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_uint16(user, data, obj);
}

static inline int template_callback_uint32(
    unpack_user* user, uint32_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_uint32(user, data, obj);
}

static inline int template_callback_uint64(
    unpack_user* user, uint64_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_uint64(user, data, obj);
}

static inline int template_callback_int8(
    unpack_user* user, int8_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_int8(user, data, obj);
}

static inline int template_callback_int16(
    unpack_user* user, int16_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_int16(user, data, obj);
}

static inline int template_callback_int32(
    unpack_user* user, int32_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_int32(user, data, obj);
}

static inline int template_callback_int64(
    unpack_user* user, int64_t data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_int64(user, data, obj);
}

static inline int template_callback_float(
    unpack_user* user, float data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_float(user, data, obj);
}

static inline int template_callback_double(
    unpack_user* user, double data, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_double(user, data, obj);
}

static inline int template_callback_nil(
    unpack_user* user, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_nil(user, obj);
}

static inline int template_callback_true(
    unpack_user* user, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_true(user, obj);
}

static inline int template_callback_false(
    unpack_user* user, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_false(user, obj);
}

static inline int template_callback_raw(
    unpack_user* user, const char* base, const char* data,
    unsigned int len, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_raw(user, base, data, len, obj);
}

static inline int template_callback_array(
    unpack_user* user, unsigned int count, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_array(user, count, obj);
}

static inline int template_callback_array_item(
    unpack_user* user, msgpack_unpack_object* container,
    msgpack_unpack_object obj)
{
    return msgpack_unserialize_array_item(user, container, obj);
}

static inline int template_callback_map(
    unpack_user* user, unsigned int count, msgpack_unpack_object* obj)
{
    return msgpack_unserialize_map(user, count, obj);
}

static inline int template_callback_map_item(
    unpack_user* user, msgpack_unpack_object* container,
    msgpack_unpack_object key, msgpack_unpack_object val)
{
    return msgpack_unserialize_map_item(user, container, key, val);
}

#include "msgpack/unpack_template.h"

#endif
