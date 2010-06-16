

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_incomplete_class.h"
#include "ext/standard/php_var.h"

#include "php_msgpack.h"
#include "msgpack_pack.h"
#include "msgpack_unpack.h"

#include "msgpack/unpack_define.h"

#define VAR_ENTRIES_MAX 1024

typedef struct
{
    zval *data[VAR_ENTRIES_MAX];
    long used_slots;
    void *next;
} var_entries;

inline static void msgpack_var_push(
    php_unserialize_data_t *var_hashx, zval **rval)
{
    var_entries *var_hash, *prev = NULL;

    if (!var_hashx)
    {
        return;
    }

    var_hash = var_hashx->first;

    while (var_hash && var_hash->used_slots == VAR_ENTRIES_MAX)
    {
        prev = var_hash;
        var_hash = var_hash->next;
    }

    if (!var_hash)
    {
        var_hash = emalloc(sizeof(var_entries));
        var_hash->used_slots = 0;
        var_hash->next = 0;

        if (!var_hashx->first)
        {
            var_hashx->first = var_hash;
        }
        else
        {
            prev->next = var_hash;
        }
    }

    var_hash->data[var_hash->used_slots++] = *rval;
}

inline static int msgpack_var_access(
    php_unserialize_data_t *var_hashx, long id, zval ***store)
{
    var_entries *var_hash = var_hashx->first;

    while (id >= VAR_ENTRIES_MAX &&
           var_hash && var_hash->used_slots == VAR_ENTRIES_MAX)
    {
        var_hash = var_hash->next;
        id -= VAR_ENTRIES_MAX;
    }

    if (!var_hash)
    {
        return !SUCCESS;
    }

    if (id < 0 || id >= var_hash->used_slots)
    {
        return !SUCCESS;
    }

    *store = &var_hash->data[id];

    return SUCCESS;
}


inline static int msgpack_unserialize_array(
    zval **return_value, const unsigned char* data, size_t len,
    size_t *off, ulong ct, php_unserialize_data_t *var_hash TSRMLS_DC)
{
    const unsigned char* p = (unsigned char*)data + *off;
    ulong i;
    HashTable *ht;

    msgpack_var_push(var_hash, return_value);

    array_init(*return_value);

    ht = HASH_OF(*return_value);

    for (i = 0; i < ct; i++)
    {
        size_t read = 0;
        zval *val;

        /* value */
        read = 0;
        ALLOC_INIT_ZVAL(val);

        if (len <= 0 ||
            msgpack_unserialize_zval(
                &val, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zend_error(E_WARNING,
                       "[msgpack] (msgpack_unserialize_array) Invalid value");

            zval_ptr_dtor(&val);

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        p += read;
        len -= read;

        /* update */
        zend_hash_index_update(ht, i, &val, sizeof(zval *), NULL);
    }

    *off = p - (const unsigned char*)data;

    return MSGPACK_UNPACK_SUCCESS;
}

inline static int msgpack_unserialize_object(
    zval **return_value, const unsigned char* data, size_t len,
    size_t *off, ulong ct, php_unserialize_data_t *var_hash TSRMLS_DC)
{
    const unsigned char* p = (unsigned char*)data;

    int ret = MSGPACK_UNPACK_SUCCESS;

    zend_class_entry *ce, **pce;
    bool incomplete_class = false;
    zval f, *h = NULL;
    zval *user_func, *retval_ptr, **args[1], *arg_func_name;
    HashTable *ht;

    size_t read = 0;
    zval *key, *val;

    int object = 1;
    int custom_object = 0;

    /* Get class */
    ALLOC_INIT_ZVAL(key);

    if (len <= 0 ||
        msgpack_unserialize_zval(&key, p, len, &read, NULL TSRMLS_CC) < 0)
    {
        zend_error(E_WARNING,
                   "[msgpack] (msgpack_unserialize_object) Invalid sign key");

        zval_ptr_dtor(&key);

        ZVAL_BOOL(*return_value, 0);

        return MSGPACK_UNPACK_PARSE_ERROR;
    }

    p += read;
    len -= read;

    ct--;

    if (Z_TYPE_P(key) == IS_NULL)
    {
        read = 0;
        ALLOC_INIT_ZVAL(val);

        if (len <= 0 ||
            msgpack_unserialize_zval(
                &val, p, len, &read, NULL TSRMLS_CC) < 0)
        {
            zend_error(
                E_WARNING,
                "[msgpack] (msgpack_unserialize_object) Invalid sign value");

            zval_ptr_dtor(&val);
            zval_ptr_dtor(&key);

            ZVAL_BOOL(*return_value, 0);

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        p += read;
        len -= read;

        if (Z_TYPE_P(val) == IS_LONG)
        {
            switch (Z_LVAL_P(val))
            {
                case MSGPACK_SERIALIZE_TYPE_REFERENCE:
                {
                    zval **rval;

                    zval_ptr_dtor(&key);
                    zval_ptr_dtor(&val);

                    read = 0;
                    ALLOC_INIT_ZVAL(key);

                    if (len <= 0 ||
                        msgpack_unserialize_zval(
                            &key, p, len, &read, NULL TSRMLS_CC) < 0)
                    {
                        zval_ptr_dtor(&key);

                        ZVAL_BOOL(*return_value, 0);

                        return MSGPACK_UNPACK_PARSE_ERROR;
                    }

                    p += read;
                    len -= read;

                    read = 0;
                    ALLOC_INIT_ZVAL(val);

                    if (len <= 0 ||
                        msgpack_unserialize_zval(
                            &val, p, len, &read, NULL TSRMLS_CC) < 0)
                    {
                        zval_ptr_dtor(&val);
                        zval_ptr_dtor(&key);

                        ZVAL_BOOL(*return_value, 0);

                        return MSGPACK_UNPACK_PARSE_ERROR;
                    }

                    p += read;
                    len -= read;

                    if (!var_hash ||
                        msgpack_var_access(
                            var_hash, Z_LVAL_P(val) - 1, &rval) != SUCCESS)
                    {
                        zend_error(E_WARNING,
                                   "[msgpack] (msgpack_unserialize_object) "
                                   "Invalid references value: %ld",
                                   Z_LVAL_P(val) - 1);

                        ret = MSGPACK_UNPACK_CONTINUE;
                    }
                    else
                    {
                        if (*return_value != NULL)
                        {
                            zval_ptr_dtor(return_value);
                        }

                        *return_value = *rval;

                        Z_ADDREF_PP(return_value);
                        Z_SET_ISREF_PP(return_value);
                    }

                    zval_ptr_dtor(&key);
                    zval_ptr_dtor(&val);

                    *off = p - (const unsigned char*)data;

                    return ret;
                }
                case MSGPACK_SERIALIZE_TYPE_OBJECT:
                {
                    zval **rval;

                    zval_ptr_dtor(&key);
                    zval_ptr_dtor(&val);

                    read = 0;
                    ALLOC_INIT_ZVAL(key);

                    if (len <= 0 ||
                        msgpack_unserialize_zval(
                            &key, p, len, &read, NULL TSRMLS_CC) < 0)
                    {
                        zval_ptr_dtor(&key);

                        ZVAL_BOOL(*return_value, 0);

                        return MSGPACK_UNPACK_PARSE_ERROR;
                    }

                    p += read;
                    len -= read;

                    read = 0;
                    ALLOC_INIT_ZVAL(val);

                    if (len <= 0 ||
                        msgpack_unserialize_zval(
                            &val, p, len, &read, var_hash TSRMLS_CC) < 0)
                    {
                        zval_ptr_dtor(&val);
                        zval_ptr_dtor(&key);

                        ZVAL_BOOL(*return_value, 0);

                        return MSGPACK_UNPACK_PARSE_ERROR;
                    }

                    p += read;
                    len -= read;

                    if (!var_hash ||
                        msgpack_var_access(
                            var_hash, Z_LVAL_P(val) - 1, &rval) != SUCCESS)
                    {
                        zend_error(E_WARNING,
                                   "[msgpack] (msgpack_unserialize_object) "
                                   "Invalid references value: %ld",
                                   Z_LVAL_P(val) - 1);

                        ret = MSGPACK_UNPACK_CONTINUE;
                    }
                    else
                    {
                        if (*return_value != *rval)
                        {
                            if (*return_value != NULL)
                            {
                                zval_ptr_dtor(return_value);
                            }

                            *return_value = *rval;

                            Z_ADDREF_PP(return_value);

                            Z_UNSET_ISREF_PP(return_value);
                        }
                    }

                    zval_ptr_dtor(&key);
                    zval_ptr_dtor(&val);

                    *off = p - (const unsigned char*)data;

                    return ret;
                }
                case MSGPACK_SERIALIZE_TYPE_CUSTOM_OBJECT:
                    custom_object = 1;

                    zval_ptr_dtor(&val);

                    read = 0;
                    ALLOC_INIT_ZVAL(val);

                    if (len <= 0 ||
                        msgpack_unserialize_zval(
                            &val, p, len, &read, NULL TSRMLS_CC) < 0)
                    {
                        zval_ptr_dtor(&key);
                        zval_ptr_dtor(&val);

                        ZVAL_BOOL(*return_value, 0);

                        return MSGPACK_UNPACK_PARSE_ERROR;
                    }

                    p += read;
                    len -= read;
                    break;
                default:
                    zval_ptr_dtor(&key);
                    zval_ptr_dtor(&val);

                    ZVAL_BOOL(*return_value, 0);

                    return MSGPACK_UNPACK_PARSE_ERROR;
            }
        }
    }
    else
    {
        object = 0;

        msgpack_var_push(var_hash, return_value);

        array_init(*return_value);

        ht = HASH_OF(*return_value);

        read = 0;
        ALLOC_INIT_ZVAL(val);

        if (len <= 0 ||
            msgpack_unserialize_zval(
                &val, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zend_error(
                E_WARNING,
                "[msgpack] (msgpack_unserialize_object) Invalid sign value");

            zval_ptr_dtor(&val);
            zval_ptr_dtor(&key);

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        p += read;
        len -= read;

        /* update */
        switch (Z_TYPE_P(key))
        {
            case IS_LONG:
                zend_hash_index_update(
                    ht, Z_LVAL_P(key), &val, sizeof(zval *), NULL);
                break;
            case IS_STRING:
                zend_symtable_update(
                    ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1,
                    &val, sizeof(zval *), NULL);
                break;
            default:
                zval_ptr_dtor(&key);
                zval_ptr_dtor(&val);

                return MSGPACK_UNPACK_PARSE_ERROR;
        }
    }

    zval_ptr_dtor(&key);

    if (object)
    {
        convert_to_string(val);

        *off = p - (const unsigned char*)data;

        do {
            /* Try to find class directly */
            if (zend_lookup_class(
                    Z_STRVAL_P(val), Z_STRLEN_P(val), &pce TSRMLS_CC) == SUCCESS)
            {
                ce = *pce;
                break;
            }

            /* Check for unserialize callback */
            if ((PG(unserialize_callback_func) == NULL) ||
                (PG(unserialize_callback_func)[0] == '\0'))
            {
                incomplete_class = 1;
                ce = PHP_IC_ENTRY;
                break;
            }

            /* Call unserialize callback */
            ALLOC_INIT_ZVAL(user_func);
            ZVAL_STRING(user_func, PG(unserialize_callback_func), 1);
            args[0] = &arg_func_name;
            ALLOC_INIT_ZVAL(arg_func_name);
            ZVAL_STRING(arg_func_name, Z_STRVAL_P(val), 1);
            if (call_user_function_ex(
                    CG(function_table), NULL, user_func, &retval_ptr,
                    1, args, 0, NULL TSRMLS_CC) != SUCCESS)
            {
                zend_error(E_WARNING,
                           "[msgpack] (msgpack_unserialize_object) "
                           "defined (%s) but not found", Z_STRVAL_P(val));

                incomplete_class = 1;
                ce = PHP_IC_ENTRY;
                zval_ptr_dtor(&user_func);
                zval_ptr_dtor(&arg_func_name);
                break;
            }
            if (retval_ptr)
            {
                zval_ptr_dtor(&retval_ptr);
            }

            /* The callback function may have defined the class */
            if (zend_lookup_class(
                    Z_STRVAL_P(val), Z_STRLEN_P(val), &pce TSRMLS_CC) == SUCCESS)
            {
                ce = *pce;
            }
            else
            {
                zend_error(
                    E_WARNING,
                    "[msgpack] (msgpack_unserialize_object) "
                    "Function %s() hasn't defined the class"
                    "it was called for", Z_STRVAL_P(val));

                incomplete_class = true;
                ce = PHP_IC_ENTRY;
            }

            zval_ptr_dtor(&user_func);
            zval_ptr_dtor(&arg_func_name);
        } while(0);

        if (EG(exception))
        {
            zend_error(
                E_WARNING,
                "[msgpack] (msgpack_unserialize_object) Exception error");

            zval_ptr_dtor(&val);

            ZVAL_BOOL(*return_value, 0);

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        msgpack_var_push(var_hash, return_value);

        object_init_ex(*return_value, ce);

        /* store incomplete class name */
        if (incomplete_class)
        {
            php_store_class_name(
                *return_value, Z_STRVAL_P(val), Z_STRLEN_P(val));
        }

        zval_ptr_dtor(&val);

        /* implementing Serializable */
        if (custom_object)
        {
            zval *rval;

            if (ce->unserialize == NULL)
            {
                zend_error(E_WARNING,
                           "[msgpack] (msgpack_unserialize_object) "
                           "Class %s has no unserializer", ce->name);

                return MSGPACK_UNPACK_PARSE_ERROR;
            }

            /* value */
            read = 0;
            ALLOC_INIT_ZVAL(rval);

            if (len <= 0 ||
                msgpack_unserialize_zval(
                    &rval, p, len, &read, var_hash TSRMLS_CC) < 0)
            {
                zval_ptr_dtor(&rval);

                return MSGPACK_UNPACK_PARSE_ERROR;
            }

            p += read;
            len -= read;

            ce->unserialize(
                return_value, ce,
                (const unsigned char *)Z_STRVAL_P(rval), Z_STRLEN_P(rval) + 1,
                (zend_unserialize_data *)var_hash TSRMLS_CC);

            zval_ptr_dtor(&key);
            zval_ptr_dtor(&rval);

            *off = p - (const unsigned char*)data;

            return ret;
        }

        ht = HASH_OF(*return_value);
    }

    /* object property */
    while (ct-- > 0)
    {
        size_t read = 0;
        zval *rval;

        /* key */
        ALLOC_INIT_ZVAL(key);

        if (len <= 0 ||
            msgpack_unserialize_zval(
                &key, p, len, &read, NULL TSRMLS_CC) < 0)
        {
            zval_ptr_dtor(&key);

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        p += read;
        len -= read;

        /* value */
        read = 0;
        ALLOC_INIT_ZVAL(rval);

        if (len <= 0 ||
            msgpack_unserialize_zval(
                &rval, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zval_ptr_dtor(&rval);
            zval_ptr_dtor(&key);

            return MSGPACK_UNPACK_PARSE_ERROR;

        }

        p += read;
        len -= read;

        /* update */
        switch (Z_TYPE_P(key))
        {
            case IS_LONG:
                zend_hash_index_update(
                    ht, Z_LVAL_P(key), &rval, sizeof(zval *), NULL);
                break;
            case IS_STRING:
                zend_symtable_update(
                    ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1,
                    &rval, sizeof(zval *), NULL);
                break;
            default:
                zval_ptr_dtor(&key);
                zval_ptr_dtor(&rval);

                return MSGPACK_UNPACK_PARSE_ERROR;
        }

        zval_ptr_dtor(&key);
    }

    /* wakeup */
    if (object && Z_OBJCE_PP(return_value) != PHP_IC_ENTRY &&
        zend_hash_exists(&Z_OBJCE_PP(return_value)->function_table,
                         "__wakeup", sizeof("__wakeup")))
    {
        INIT_PZVAL(&f);
        ZVAL_STRINGL(&f, "__wakeup", sizeof("__wakeup") - 1, 0);
        call_user_function_ex(
            CG(function_table), return_value, &f, &h, 0, 0, 1, NULL TSRMLS_CC);
        if (h)
        {
            zval_ptr_dtor(&h);
        }

        if (EG(exception))
        {
            ret = MSGPACK_UNPACK_PARSE_ERROR;
        }
    }

    *off = p - (const unsigned char*)data;

    return ret;
}

PHP_MSGPACK_API int msgpack_unserialize_zval(
    zval **return_value, const unsigned char* data,
    size_t len, size_t* off, php_unserialize_data_t *var_hash TSRMLS_DC)
{
    const unsigned char* p = (unsigned char*)data + *off;
    const unsigned char* const pe = (unsigned char*)data + len;
    const void* n = NULL;

    unsigned int trail = 0;
    unsigned int cs = CS_HEADER;

    int ret;
    unsigned int ct;

    if (p == pe)
    {
        goto _out;
    }
    do
    {
        switch (cs)
        {
            case CS_HEADER:
                switch (*p)
                {
                    case 0x00 ... 0x7f: /* Positive Fixnum */
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, *(uint8_t*)p);
                        goto _finish;
                    case 0xe0 ... 0xff: /* Negative Fixnum */
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, *(int8_t*)p);
                        goto _finish;
                    case 0xc0 ... 0xdf: /* Variable */
                        switch (*p)
                        {
                            case 0xc0:  /* nil */
                                msgpack_var_push(var_hash, return_value);
                                ZVAL_NULL(*return_value);
                                goto _finish;
                            case 0xc2:  /* false */
                                msgpack_var_push(var_hash, return_value);
                                ZVAL_BOOL(*return_value, 0);
                                goto _finish;
                            case 0xc3:  /* true */
                                msgpack_var_push(var_hash, return_value);
                                ZVAL_BOOL(*return_value, 1);
                                goto _finish;
                            case 0xca:  /* float */
                            case 0xcb:  /* double */
                            case 0xcc:  /* unsigned int  8 */
                            case 0xcd:  /* unsigned int 16 */
                            case 0xce:  /* unsigned int 32 */
                            case 0xcf:  /* unsigned int 64 */
                            case 0xd0:  /* signed int  8 */
                            case 0xd1:  /* signed int 16 */
                            case 0xd2:  /* signed int 32 */
                            case 0xd3:  /* signed int 64 */
                                trail = 1 << (((unsigned int)*p) & 0x03);
                                cs = ((unsigned int)*p & 0x1f);
                                goto _fixed_trail_again;
                            case 0xda:  /* raw 16 */
                            case 0xdb:  /* raw 32 */
                            case 0xdc:  /* array 16 */
                            case 0xdd:  /* array 32 */
                            case 0xde:  /* map 16 */
                            case 0xdf:  /* map 32 */
                                trail = 2 << (((unsigned int)*p) & 0x01);
                                cs = ((unsigned int)*p & 0x1f);
                                goto _fixed_trail_again;
                            default:
                                goto _failed;
                        }
                    case 0xa0 ... 0xbf: /* FixRaw */
                        trail = ((unsigned int)*p & 0x1f);
                        if (trail == 0)
                        {
                            goto _raw_zero;
                        }
                        cs = ACS_RAW_VALUE;
                        goto _fixed_trail_again;
                    case 0x90 ... 0x9f: /* FixArray */
                    {
                        size_t read = 0;
                        ct = (((unsigned int)*p) & 0x0f);
                        cs = CS_HEADER;
                        ++p;
                        ret = msgpack_unserialize_array(
                            return_value, p, pe - p, &read,
                            ct, var_hash TSRMLS_CC);
                        p += read;
                        if (ret < 0)
                        {
                            goto _failed;
                        }
                        --p;
                        goto _finish;
                    }
                    case 0x80 ... 0x8f: /* FixMap */
                    {
                        size_t read = 0;
                        ct = (((unsigned int)*p) & 0x0f);
                        cs = CS_HEADER;
                        ++p;
                        ret = msgpack_unserialize_object(
                            return_value, p, pe - p, &read,
                            ct, var_hash TSRMLS_CC);
                        p += read;
                        if (ret < 0)
                        {
                            goto _failed;
                        }
                        --p;
                        goto _finish;
                    }
                    default:
                        goto _failed;
                }
_fixed_trail_again:
                ++p;
            default:
                if ((size_t)(pe - p) < trail)
                {
                    goto _out;
                }
                n = p;
                p += trail - 1;
                switch (cs)
                {
                    case CS_FLOAT:
                    {
                        union { uint32_t i; float f; } mem;
                        mem.i = _msgpack_load32(uint32_t, n);
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_DOUBLE(*return_value, mem.f);
                        goto _finish;
                    }
                    case CS_DOUBLE:
                    {
                        union { uint64_t i; double f; } mem;
                        mem.i = _msgpack_load64(uint64_t, n);
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_DOUBLE(*return_value, mem.f);
                        goto _finish;
                    }
                    case CS_UINT_8:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, *(uint8_t*)n);
                        goto _finish;
                    case CS_UINT_16:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, _msgpack_load16(uint16_t, n));
                        goto _finish;
                    case CS_UINT_32:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, _msgpack_load32(uint32_t, n));
                        goto _finish;
                    case CS_UINT_64:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, _msgpack_load64(uint64_t, n));
                        goto _finish;
                    case CS_INT_8:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, *(int8_t*)n);
                        goto _finish;
                    case CS_INT_16:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, _msgpack_load16(int16_t, n));
                        goto _finish;
                    case CS_INT_32:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, _msgpack_load32(int32_t, n));
                        goto _finish;
                    case CS_INT_64:
                        msgpack_var_push(var_hash, return_value);
                        ZVAL_LONG(*return_value, _msgpack_load64(int64_t, n));
                        goto _finish;
                    case CS_RAW_16:
                        trail = _msgpack_load16(uint16_t, n);
                        if (trail == 0) {
                            goto _raw_zero;
                        }
                        cs = ACS_RAW_VALUE;
                        goto _fixed_trail_again;
                    case CS_RAW_32:
                        trail = _msgpack_load32(uint32_t, n);
                        if (trail == 0) {
                            goto _raw_zero;
                        }
                        cs = ACS_RAW_VALUE;
                        goto _fixed_trail_again;
                    case ACS_RAW_VALUE:
_raw_zero:
                        msgpack_var_push(var_hash, return_value);
                        if (trail == 0)
                        {
                            ZVAL_STRINGL(*return_value, "", 0, 1);
                        }
                        else
                        {
                            ZVAL_STRINGL(*return_value, n, trail, 1);
                        }
                        goto _finish;
                    case CS_ARRAY_16:
                    {
                        size_t read = 0;
                        ct = _msgpack_load16(uint16_t, n);
                        cs = CS_HEADER;
                        ++p;
                        ret = msgpack_unserialize_array(
                            return_value, p, pe - p, &read,
                            ct, var_hash TSRMLS_CC);
                        p += read;
                        if (ret < 0)
                        {
                            goto _failed;
                        }
                        --p;
                        goto _finish;
                    }
                    case CS_ARRAY_32:
                    {
                        size_t read = 0;
                        ct = _msgpack_load32(uint32_t, n);
                        cs = CS_HEADER;
                        ++p;
                        ret = msgpack_unserialize_array(
                            return_value, p, pe - p, &read,
                            ct, var_hash TSRMLS_CC);
                        p += read;
                        if (ret < 0)
                        {
                            goto _failed;
                        }
                        --p;
                        goto _finish;
                    }
                    /* FIXME security guard */
                    case CS_MAP_16:
                    {
                        size_t read = 0;
                        ct = _msgpack_load16(uint16_t, n);
                        cs = CS_HEADER;
                        ++p;
                        ret = msgpack_unserialize_object(
                            return_value, p, pe - p, &read,
                            ct, var_hash TSRMLS_CC);
                        p += read;
                        if (ret < 0)
                        {
                            goto _failed;
                        }
                        --p;
                        goto _finish;
                    }
                    case CS_MAP_32:
                    {
                        size_t read = 0;
                        ct = _msgpack_load32(uint32_t, n);
                        read = 0;
                        cs = CS_HEADER;
                        ++p;
                        ret = msgpack_unserialize_object(
                            return_value, p, pe - p, &read,
                            ct, var_hash TSRMLS_CC);
                        p += read;
                        if (ret < 0)
                        {
                            goto _failed;
                        }
                        --p;
                        goto _finish;
                    }
                    /* FIXME security guard */
                    default:
                        goto _failed;
                }
        }
        cs = CS_HEADER;
        ++p;
    }
    while (p != pe);
    goto _out;

_finish:
    ++p;
    ret = MSGPACK_UNPACK_EXTRA_BYTES;
    goto _end;

_failed:
    ret = MSGPACK_UNPACK_PARSE_ERROR;
    goto _end;

_out:
    ret = MSGPACK_UNPACK_CONTINUE;
    goto _end;

_end:
    *off = p - (const unsigned char*)data;

    if (ret == MSGPACK_UNPACK_EXTRA_BYTES && len == *off)
    {
        ret = MSGPACK_UNPACK_SUCCESS;
    }

    return ret;
}
