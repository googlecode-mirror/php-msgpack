
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_incomplete_class.h"
#include "ext/standard/php_var.h"
#include "ext/session/php_session.h"

#include "php_msgpack.h"
#include "msgpack_pack.h"
#include "msgpack_unpack.h"

static PHP_FUNCTION(msgpack_serialize);
static PHP_FUNCTION(msgpack_unserialize);

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_serialize, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_unserialize, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

PS_SERIALIZER_FUNCS(msgpack);

static const zend_function_entry msgpack_functions[] = {
    PHP_FE(msgpack_serialize, arginfo_msgpack_serialize)
    PHP_FE(msgpack_unserialize, arginfo_msgpack_unserialize)
    {NULL, NULL, NULL}
};

static PHP_MINIT_FUNCTION(msgpack)
{
#if HAVE_PHP_SESSION
    php_session_register_serializer("msgpack",
                                    PS_SERIALIZER_ENCODE_NAME(msgpack),
                                    PS_SERIALIZER_DECODE_NAME(msgpack));
#endif
    return SUCCESS;
}

static PHP_MINFO_FUNCTION(msgpack)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "msgpack support", "enabled");
    php_info_print_table_row(2, "msgpack version", MSGPACK_VERSION);
#if HAVE_PHP_SESSION
    php_info_print_table_row(2, "msgpack Session Support", "enabled" );
#endif
    php_info_print_table_end();
}

zend_module_entry msgpack_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "msgpack",
    msgpack_functions,
    PHP_MINIT(msgpack),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(msgpack),
#if ZEND_MODULE_API_NO >= 20010901
    MSGPACK_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_MSGPACK
ZEND_GET_MODULE(msgpack)
#endif

inline static void msgpack_serialize_zval(
    smart_str *buf, zval *val, HashTable *var_hash TSRMLS_DC);
inline static void msgpack_serialize_string(
    smart_str *buf, char *str, size_t len);
inline static void msgpack_serialize_class(
    smart_str *buf, zval *val, zval *retval_ptr, HashTable *var_hash,
    char *class_name, zend_uint name_len, zend_bool incomplete_class TSRMLS_DC);
inline static void msgpack_serialize_array(
    smart_str *buf, zval *val, HashTable *var_hash, bool object,
    char *class_name, zend_uint name_len, zend_bool incomplete_class TSRMLS_DC);
inline static void msgpack_serialize_object(
    smart_str *buf, zval *val, HashTable *var_has,
    char *class_name, zend_uint name_len, zend_bool incomplete_class TSRMLS_DC);

inline static int msgpack_unserialize_zval(
    zval **return_value, const unsigned char *data,
    size_t len, size_t *off, php_unserialize_data_t *var_hash TSRMLS_DC);
inline static int msgpack_unserialize_array(
    zval **return_value, const unsigned char *data, size_t len,
    size_t *off, ulong ct, php_unserialize_data_t *var_hash TSRMLS_DC);
inline static int msgpack_unserialize_object(
    zval **return_value, const unsigned char *data, size_t len,
    size_t *off, ulong ct, php_unserialize_data_t *var_hash TSRMLS_DC);

PS_SERIALIZER_ENCODE_FUNC(msgpack)
{
    smart_str buf = {0};
    php_serialize_data_t var_hash;

    PHP_VAR_SERIALIZE_INIT(var_hash);

    msgpack_serialize_array(
        &buf, PS(http_session_vars), &var_hash, false, NULL, 0, 0 TSRMLS_CC);

    if (newlen)
    {
        *newlen = buf.len;
    }

    smart_str_0(&buf);
    *newstr = buf.c;

    PHP_VAR_SERIALIZE_DESTROY(var_hash);

    return SUCCESS;
}

PS_SERIALIZER_DECODE_FUNC(msgpack)
{
    php_unserialize_data_t var_hash;
    int ret;
    size_t off = 0;
    HashTable *tmp_hash;
    HashPosition tmp_hash_pos;
    char *key_str;
    ulong key_long;
    uint key_len;
    zval *tmp;
    zval **data;

    PHP_VAR_UNSERIALIZE_INIT(var_hash);

    MAKE_STD_ZVAL(tmp);

    ret = msgpack_unserialize_zval(
        &tmp, (const unsigned char *)val, vallen,
        &off, &var_hash TSRMLS_CC);

    switch (ret)
    {
        case MSGPACK_UNPACK_EXTRA_BYTES:
        case MSGPACK_UNPACK_SUCCESS:
            break;
        case MSGPACK_UNPACK_PARSE_ERROR:
        case MSGPACK_UNPACK_CONTINUE:
        default:
            zval_ptr_dtor(&tmp);
            return FAILURE;
    }

    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

    tmp_hash = HASH_OF(tmp);

    zend_hash_internal_pointer_reset_ex(tmp_hash, &tmp_hash_pos);
    while (zend_hash_get_current_data_ex(
               tmp_hash, (void *)&data, &tmp_hash_pos) == SUCCESS)
    {
        ret = zend_hash_get_current_key_ex(
            tmp_hash, &key_str, &key_len, &key_long, 0, &tmp_hash_pos);
        switch (ret)
        {
            case HASH_KEY_IS_LONG:
                /* ??? */
                break;
            case HASH_KEY_IS_STRING:
                php_set_session_var(key_str, key_len - 1, *data, NULL TSRMLS_CC);
                php_add_session_var(key_str, key_len - 1 TSRMLS_CC);
                break;
        }
        zend_hash_move_forward_ex(tmp_hash, &tmp_hash_pos);
    }

    zval_ptr_dtor(&tmp);

    return SUCCESS;
}

inline static int msgpack_var_add(
    HashTable *var_hash, zval *var, void *var_old TSRMLS_DC)
{
    ulong var_no;
    char id[32], *p;
    int len;

    if ((Z_TYPE_P(var) == IS_OBJECT) && Z_OBJ_HT_P(var)->get_class_entry)
    {
        p = smart_str_print_long(
            id + sizeof(id) - 1,
            (((size_t)Z_OBJCE_P(var) << 5)
             | ((size_t)Z_OBJCE_P(var) >> (sizeof(long) * 8 - 5)))
            + (long)Z_OBJ_HANDLE_P(var));
        len = id + sizeof(id) - 1 - p;
    }
    else
    {
        p = smart_str_print_long(id + sizeof(id) - 1, (long)var);
        len = id + sizeof(id) - 1 - p;
    }

    if (var_old && zend_hash_find(var_hash, p, len, var_old) == SUCCESS)
    {
        if (!Z_ISREF_P(var))
        {
            var_no = -1;
            zend_hash_next_index_insert(
                var_hash, &var_no, sizeof(var_no), NULL);
        }
        return FAILURE;
    }

    var_no = zend_hash_num_elements(var_hash) + 1;

    zend_hash_add(var_hash, p, len, &var_no, sizeof(var_no), NULL);

    return SUCCESS;
}

inline static void msgpack_var_push(
    php_unserialize_data_t *var_hashx, zval **rval)
{
    var_entries *var_hash = var_hashx->first, *prev = NULL;

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
        return -1;
    }

    if (id < 0 || id >= var_hash->used_slots)
    {
        return -1;
    }

    *store = &var_hash->data[id];

    return 0;
}

inline static void msgpack_serialize_zval(
    smart_str *buf, zval *val, HashTable *var_hash TSRMLS_DC)
{
    ulong *var_already;

    if (var_hash &&
        msgpack_var_add(
            var_hash, val, (void *)&var_already TSRMLS_CC) == FAILURE)
    {
        if (Z_ISREF_P(val) || Z_TYPE_P(val) == IS_OBJECT)
        {
            msgpack_pack_map(buf, 1);
            msgpack_serialize_string(buf, "__ref", sizeof("__ref"));
            msgpack_pack_long(buf, *var_already);
            return;
        }
    }

    switch (Z_TYPE_P(val))
    {
        case IS_NULL:
            msgpack_pack_nil(buf);
            break;
        case IS_BOOL:
            if (Z_BVAL_P(val))
            {
                msgpack_pack_true(buf);
            }
            else
            {
                msgpack_pack_false(buf);
            }
            break;
        case IS_LONG:
            msgpack_pack_long(buf, Z_LVAL_P(val));
            break;
        case IS_DOUBLE:
            {
                double dbl = Z_DVAL_P(val);
                msgpack_pack_double(buf, dbl);
            }
            break;
        case IS_STRING:
            msgpack_serialize_string(
                buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            break;
        case IS_ARRAY:
            msgpack_serialize_array(
                buf, val, var_hash, false, NULL, 0, 0 TSRMLS_CC);
            break;
        case IS_OBJECT:
            {
                PHP_CLASS_ATTRIBUTES;
                PHP_SET_CLASS_ATTRIBUTES(val);

                msgpack_serialize_object(
                    buf, val, var_hash, class_name, name_len,
                    incomplete_class TSRMLS_CC);

                PHP_CLEANUP_CLASS_ATTRIBUTES();
            }
            break;
        default:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_serialize) "
                       "type is unsupported, encoded as null");
            msgpack_pack_nil(buf);
            break;
    }
    return;
}

inline static void msgpack_serialize_string(
    smart_str *buf, char *str, size_t len)
{
    msgpack_pack_raw(buf, len);
    msgpack_pack_raw_body(buf, str, len);
}

inline static void msgpack_serialize_class(
    smart_str *buf, zval *val, zval *retval_ptr, HashTable *var_hash,
    char *class_name, zend_uint name_len, zend_bool incomplete_class TSRMLS_DC)
{
    int count;
    HashTable *ht = HASH_OF(retval_ptr);

    count = zend_hash_num_elements(ht);
    if (incomplete_class)
    {
        --count;
    }

    if (count > 0)
    {
        char *key;
        zval **data, **name;
        ulong key_index;
        HashPosition pos;
        int n;
        zval nval, *nvalp;

        msgpack_pack_map(buf, count + 1);

        msgpack_serialize_string(buf, class_name, name_len);
        msgpack_serialize_string(buf, class_name, name_len);

        ZVAL_NULL(&nval);
        nvalp = &nval;

        zend_hash_internal_pointer_reset_ex(ht, &pos);

        for (;; zend_hash_move_forward_ex(ht, &pos))
        {
            n = zend_hash_get_current_key_ex(
                ht, &key, NULL, &key_index, 0, &pos);

            if (n == HASH_KEY_NON_EXISTANT)
            {
                break;
            }
            if (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0)
            {
                continue;
            }

            zend_hash_get_current_data_ex(ht, (void **)&name, &pos);

            if (Z_TYPE_PP(name) != IS_STRING)
            {
                zend_error(E_NOTICE,
                           "[msgpack] (msgpack_serialize_class) "
                           "__sleep should return an array only "
                           "containing the names of "
                           "instance-variables to serialize.");
                msgpack_pack_nil(buf);
                continue;
            }

            if (zend_hash_find(
                    Z_OBJPROP_P(val), Z_STRVAL_PP(name),
                    Z_STRLEN_PP(name) + 1, (void *)&data) == SUCCESS)
            {
                msgpack_serialize_string(
                    buf, Z_STRVAL_PP(name), Z_STRLEN_PP(name));
                msgpack_serialize_zval(buf, *data, var_hash TSRMLS_CC);
            }
            else
            {
                zend_class_entry *ce;
                ce = zend_get_class_entry(val TSRMLS_CC);
                if (ce)
                {
                    char *prot_name, *priv_name;
                    int prop_name_length;

                    do
                    {
                        zend_mangle_property_name(
                            &priv_name, &prop_name_length, ce->name,
                            ce->name_length, Z_STRVAL_PP(name),
                            Z_STRLEN_PP(name),
                            ce->type & ZEND_INTERNAL_CLASS);
                        if (zend_hash_find(
                                Z_OBJPROP_P(val), priv_name,
                                prop_name_length + 1,
                                (void *)&data) == SUCCESS)
                        {
                            msgpack_serialize_string(
                                buf, priv_name, prop_name_length);

                            pefree(priv_name,
                                   ce->type & ZEND_INTERNAL_CLASS);

                            msgpack_serialize_zval(
                                buf, *data, var_hash TSRMLS_CC);
                            break;
                        }
                        pefree(priv_name,
                               ce->type & ZEND_INTERNAL_CLASS);
                        zend_mangle_property_name(
                            &prot_name, &prop_name_length, "*", 1,
                            Z_STRVAL_PP(name), Z_STRLEN_PP(name),
                            ce->type & ZEND_INTERNAL_CLASS);
                        if (zend_hash_find(
                                Z_OBJPROP_P(val), prot_name,
                                prop_name_length + 1,
                                (void *)&data) == SUCCESS)
                        {
                            msgpack_serialize_string(
                                buf, prot_name, prop_name_length);

                            pefree(prot_name,
                                   ce->type & ZEND_INTERNAL_CLASS);

                            msgpack_serialize_zval(
                                buf, *data, var_hash TSRMLS_CC);
                            break;
                        }
                        pefree(prot_name, ce->type & ZEND_INTERNAL_CLASS);

                        zend_error(E_NOTICE,
                                   "[msgpack] (msgpack_serialize_class) "
                                   "\"%s\" returned as member variable from "
                                   "__sleep() but does not exist",
                                   Z_STRVAL_PP(name));

                        msgpack_serialize_string(
                            buf, Z_STRVAL_PP(name), Z_STRLEN_PP(name));

                        msgpack_serialize_zval(
                            buf, nvalp, var_hash TSRMLS_CC);
                    }
                    while (0);
                }
                else
                {
                    msgpack_serialize_string(
                        buf, Z_STRVAL_PP(name), Z_STRLEN_PP(name));

                    msgpack_serialize_zval(buf, nvalp, var_hash TSRMLS_CC);
                }
            }
        }
    }
}

inline static void msgpack_serialize_array(
    smart_str *buf, zval *val, HashTable *var_hash, bool object,
    char* class_name, zend_uint name_len, zend_bool incomplete_class TSRMLS_DC)
{
    HashTable *ht;
    size_t n;

    if (object)
    {
        ht = Z_OBJPROP_P(val);
    }
    else
    {
        ht = HASH_OF(val);
    }

    if (ht)
    {
        n = zend_hash_num_elements(ht);
    }
    else
    {
        n = 0;
    }

    if (n > 0 && incomplete_class)
    {
        --n;
    }

    if (object)
    {
        msgpack_pack_map(buf, n + 1);
        msgpack_serialize_string(buf, "__class", sizeof("__class"));
        msgpack_serialize_string(buf, class_name, name_len);
    }
    else
    {
        msgpack_pack_array(buf, n * 2);
    }

    if (n > 0)
    {
        char *key;
        uint key_len;
        int key_type;
        ulong key_index;
        zval **data;
        HashPosition pos;

        zend_hash_internal_pointer_reset_ex(ht, &pos);
        for (;; zend_hash_move_forward_ex(ht, &pos))
        {
            key_type = zend_hash_get_current_key_ex(
                ht, &key, &key_len, &key_index, 0, &pos);

            if (key_type == HASH_KEY_NON_EXISTANT)
            {
                break;
            }
            if (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0)
            {
                continue;
            }

            switch (key_type)
            {
                case HASH_KEY_IS_LONG:
                    msgpack_pack_long(buf, key_index);
                    break;
                case HASH_KEY_IS_STRING:
                    msgpack_serialize_string(buf, key, key_len - 1);
                    break;
                default:
                    msgpack_serialize_string(buf, "", sizeof(""));
                    zend_error(E_WARNING, "[msgpack] (msgpack_serialize_array) "
                               "key is not string nor array");
                    break;
            }

            if (zend_hash_get_current_data_ex(
                    ht, (void *)&data, &pos) != SUCCESS ||
                !data || data == &val ||
                (Z_TYPE_PP(data) == IS_ARRAY &&
                 Z_ARRVAL_PP(data)->nApplyCount > 1))
            {
                msgpack_pack_nil(buf);
            }
            else
            {
                if (Z_TYPE_PP(data) == IS_ARRAY)
                {
                    Z_ARRVAL_PP(data)->nApplyCount++;
                }

                msgpack_serialize_zval(buf, *data, var_hash TSRMLS_CC);

                if (Z_TYPE_PP(data) == IS_ARRAY)
                {
                    Z_ARRVAL_PP(data)->nApplyCount--;
                }
            }
        }
    }
}

inline static void msgpack_serialize_object(
    smart_str *buf, zval *val, HashTable *var_hash,
    char* class_name, zend_uint name_len, zend_bool incomplete_class TSRMLS_DC)
{
    zval *retval_ptr = NULL;
    zval fname;
    int res;
    zend_class_entry *ce = NULL;

    if (Z_OBJ_HT_P(val)->get_class_entry)
    {
        ce = Z_OBJCE_P(val);
    }

    if (ce && ce->serialize != NULL)
    {
        unsigned char *serialized_data = NULL;
        zend_uint serialized_length;

        if (ce->serialize(
                val, &serialized_data, &serialized_length,
                (zend_serialize_data *)var_hash TSRMLS_CC) == SUCCESS &&
            !EG(exception))
        /*
        if (ce->serialize(
                val, &serialized_data, &serialized_len,
                (zend_serialize_data *)NULL TSRMLS_CC) == SUCCESS &&
            !EG(exception))
        */
        {
            msgpack_pack_map(buf, 2);

            msgpack_serialize_string(buf, ce->name, ce->name_length);

            msgpack_pack_raw(buf, serialized_length);
            msgpack_pack_raw_body(buf, serialized_data, serialized_length);
        }
        else
        {
            msgpack_pack_nil(buf);
        }

        if (serialized_data)
        {
            efree(serialized_data);
        }

        return;
    }

    if (ce && ce != PHP_IC_ENTRY &&
        zend_hash_exists(&ce->function_table, "__sleep", sizeof("__sleep")))
    {
        INIT_PZVAL(&fname);
        ZVAL_STRINGL(&fname, "__sleep", sizeof("__sleep") - 1, 0);
        res = call_user_function_ex(CG(function_table), &val, &fname,
                                    &retval_ptr, 0, 0, 1, NULL TSRMLS_CC);
        if (res == SUCCESS && !EG(exception))
        {
            if (retval_ptr)
            {
                if (HASH_OF(retval_ptr))
                {
                    msgpack_serialize_class(
                        buf, val, retval_ptr, var_hash,
                        class_name, name_len, incomplete_class TSRMLS_CC);
                }
                else
                {
                    zend_error(E_NOTICE,
                               "[msgpack] (msgpack_serialize_object) "
                               "__sleep should return an array only "
                               "containing the names of instance-variables "
                               "to serialize");
                    msgpack_pack_nil(buf);
                }
                zval_ptr_dtor(&retval_ptr);
            }
            return;
        }
    }

    if (retval_ptr)
    {
        zval_ptr_dtor(&retval_ptr);
    }

    msgpack_serialize_array(
        buf, val, var_hash, true,
        class_name, name_len, incomplete_class TSRMLS_CC);
}

inline static int msgpack_unserialize_zval(
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
                    case 0x00 ... 0x7f: // Positive Fixnum
                        ZVAL_LONG(*return_value, *(uint8_t*)p);
                        goto _finish;
                    case 0xe0 ... 0xff: // Negative Fixnum
                        ZVAL_LONG(*return_value, *(int8_t*)p);
                        goto _finish;
                    case 0xc0 ... 0xdf: // Variable
                        switch (*p)
                        {
                            case 0xc0:  // nil
                                ZVAL_NULL(*return_value);
                                goto _finish;
                            case 0xc2:  // false
                                ZVAL_BOOL(*return_value, 0);
                                goto _finish;
                            case 0xc3:  // true
                                ZVAL_BOOL(*return_value, 1);
                                goto _finish;
                            case 0xca:  // float
                            case 0xcb:  // double
                            case 0xcc:  // unsigned int  8
                            case 0xcd:  // unsigned int 16
                            case 0xce:  // unsigned int 32
                            case 0xcf:  // unsigned int 64
                            case 0xd0:  // signed int  8
                            case 0xd1:  // signed int 16
                            case 0xd2:  // signed int 32
                            case 0xd3:  // signed int 64
                                trail = 1 << (((unsigned int)*p) & 0x03);
                                cs = ((unsigned int)*p & 0x1f);
                                goto _fixed_trail_again;
                            case 0xda:  // raw 16
                            case 0xdb:  // raw 32
                            case 0xdc:  // array 16
                            case 0xdd:  // array 32
                            case 0xde:  // map 16
                            case 0xdf:  // map 32
                                trail = 2 << (((unsigned int)*p) & 0x01);
                                cs = ((unsigned int)*p & 0x1f);
                                goto _fixed_trail_again;
                            default:
                                goto _failed;
                        }
                    case 0xa0 ... 0xbf: // FixRaw
                        trail = ((unsigned int)*p & 0x1f);
                        if (trail == 0)
                        {
                            goto _raw_zero;
                        }
                        cs = ACS_RAW_VALUE;
                        goto _fixed_trail_again;
                    case 0x90 ... 0x9f: // FixArray
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
                    case 0x80 ... 0x8f: // FixMap
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
                        ZVAL_DOUBLE(*return_value, mem.f);
                        goto _finish;
                    }
                    case CS_DOUBLE:
                    {
                        union { uint64_t i; double f; } mem;
                        mem.i = _msgpack_load64(uint64_t, n);
                        ZVAL_DOUBLE(*return_value, mem.f);
                        goto _finish;
                    }
                    case CS_UINT_8:
                        ZVAL_LONG(*return_value, *(uint8_t*)n);
                        goto _finish;
                    case CS_UINT_16:
                        ZVAL_LONG(*return_value, _msgpack_load16(uint16_t, n));
                        goto _finish;
                    case CS_UINT_32:
                        ZVAL_LONG(*return_value, _msgpack_load32(uint32_t, n));
                        goto _finish;
                    case CS_UINT_64:
                        ZVAL_LONG(*return_value, _msgpack_load64(uint64_t, n));
                        goto _finish;
                    case CS_INT_8:
                        ZVAL_LONG(*return_value, *(int8_t*)n);
                        goto _finish;
                    case CS_INT_16:
                        ZVAL_LONG(*return_value, _msgpack_load16(int16_t, n));
                        goto _finish;
                    case CS_INT_32:
                        ZVAL_LONG(*return_value, _msgpack_load32(int32_t, n));
                        goto _finish;
                    case CS_INT_64:
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

inline static int msgpack_unserialize_array(
    zval **return_value, const unsigned char* data, size_t len,
    size_t *off, ulong ct, php_unserialize_data_t *var_hash TSRMLS_DC)
{
    const unsigned char* p = (unsigned char*)data + *off;
    ulong i;
    HashTable *ht;

    array_init(*return_value);

    msgpack_var_push(var_hash, return_value);

    if (ct % 2 != 0)
    {
        zend_error(E_WARNING,
                   "[msgpack] (msgpack_unserialize_array) Invalid count");
        return MSGPACK_UNPACK_PARSE_ERROR;
    }

    ct = ct / 2;

    ht = HASH_OF(*return_value);

    for (i = 0; i < ct; i++)
    {
        size_t read = 0;
        zval *key, *val;

        MAKE_STD_ZVAL(key);

        if (msgpack_unserialize_zval(
                &key, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zend_error(E_WARNING,
                       "[msgpack] (msgpack_unserialize_array) Invalid key");

            zval_ptr_dtor(&key);
            *off = p - (const unsigned char*)data;

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        p += read;
        len -= read;

        //value
        read = 0;
        MAKE_STD_ZVAL(val);

        if (msgpack_unserialize_zval(
                &val, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zend_error(E_WARNING,
                       "[msgpack] (msgpack_unserialize_array) Invalid value");

            zval_ptr_dtor(&val);
            zval_ptr_dtor(&key);
            *off = p - (const unsigned char*)data;

            return MSGPACK_UNPACK_PARSE_ERROR;
        }

        p += read;
        len -= read;

        //create
        switch (Z_TYPE_P(key))
        {
            case IS_LONG:
                zend_hash_index_update(
                    ht, Z_LVAL_P(key), &val, sizeof(val), NULL);
                break;
            case IS_STRING:
                zend_symtable_update(
                    ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1,
                    &val, sizeof(val), NULL);
                break;
            default:
                convert_to_string(key);
                if (Z_STRLEN_P(key) > 0)
                {
                    zend_symtable_update(
                        ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1,
                        &val, sizeof(val), NULL);
                }
                else
                {
                    zval_ptr_dtor(&val);
                }
                break;
        }

        zval_ptr_dtor(&key);
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
    ulong i;
    HashTable *ht;

    size_t read = 0;
    zval *key, *val;

    /* Get class */
    MAKE_STD_ZVAL(key);

    if (msgpack_unserialize_zval(&key, p, len, &read, var_hash TSRMLS_CC) < 0)
    {
        zend_error(E_WARNING,
                   "[msgpack] (msgpack_unserialize_object) Invalid sign key");

        zval_ptr_dtor(&key);
        return MSGPACK_UNPACK_PARSE_ERROR;
    }

    p += read;
    len -= read;

    read = 0;
    MAKE_STD_ZVAL(val);

    if (msgpack_unserialize_zval(&val, p, len, &read, var_hash TSRMLS_CC) < 0)
    {
        zend_error(E_WARNING,
                   "[msgpack] (msgpack_unserialize_object) Invalid sign value");

        zval_ptr_dtor(&val);
        zval_ptr_dtor(&key);
        *off = p - (const unsigned char*)data;
        return MSGPACK_UNPACK_PARSE_ERROR;
    }

    ct--;
    p += read;
    len -= read;
    *off = p - (const unsigned char*)data;

    if (strcmp(Z_STRVAL_P(key), "__ref") == 0)
    {
        zval **rval;

        if (!var_hash ||
            msgpack_var_access(var_hash, Z_LVAL_P(val) - 1, &rval) != SUCCESS)
        {
            /*
            zend_error(E_WARNING,
                       "[msgpack] (msgpack_unserialize_object) "
                       "Invalid references value: %ld", Z_LVAL_P(val) - 1);
            */
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

        msgpack_var_push(var_hash, return_value);

        return ret;
    }

    zval_ptr_dtor(&key);

    convert_to_string(val);

    //
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
        MAKE_STD_ZVAL(user_func);
        ZVAL_STRING(user_func, PG(unserialize_callback_func), 1);
        args[0] = &arg_func_name;
        MAKE_STD_ZVAL(arg_func_name);
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
            //warning
            zend_error(E_WARNING,
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
        zend_error(E_WARNING,
                   "[msgpack] (msgpack_unserialize_object) Exception error");

        zval_ptr_dtor(&val);
        return MSGPACK_UNPACK_PARSE_ERROR;
    }

    object_init_ex(*return_value, ce);

    msgpack_var_push(var_hash, return_value);

    /* store incomplete class name */
    if (incomplete_class)
    {
        php_store_class_name(*return_value, Z_STRVAL_P(val), Z_STRLEN_P(val));
    }

    //
    ht = HASH_OF(*return_value);
    for (i = 0; i < ct; i++)
    {
        size_t read = 0;
        zval *rval;

        //key
        MAKE_STD_ZVAL(key);

        if (msgpack_unserialize_zval(
                &key, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zval_ptr_dtor(&key);
            break;
        }

        p += read;
        len -= read;

        //value
        read = 0;
        MAKE_STD_ZVAL(rval);

        if (msgpack_unserialize_zval(
                &rval, p, len, &read, var_hash TSRMLS_CC) < 0)
        {
            zval_ptr_dtor(&rval);
            zval_ptr_dtor(&key);
            break;
        }

        p += read;
        len -= read;

        //update
        switch (Z_TYPE_P(key))
        {
            case IS_LONG:
                zend_hash_index_update(
                    ht, Z_LVAL_P(key), &rval, sizeof(rval), NULL);
                break;
            case IS_STRING:
                zend_symtable_update(
                    ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1,
                    &rval, sizeof(rval), NULL);
                break;
            default:
                convert_to_string(key);
                if (Z_STRLEN_P(key) > 0)
                {
                    zend_symtable_update(
                        ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1,
                        &rval, sizeof(rval), NULL);
                }
                else
                {
                    zval_ptr_dtor(&rval);
                }
                break;
        }
        zval_ptr_dtor(&key);
    }

    if (Z_OBJCE_PP(return_value) != PHP_IC_ENTRY &&
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

    zval_ptr_dtor(&val);

    *off = p - (const unsigned char*)data;

    return ret;
}


PHP_MSGPACK_API void php_msgpack_serialize(smart_str *buf, zval *val TSRMLS_DC)
{
    php_serialize_data_t var_hash;

    PHP_VAR_SERIALIZE_INIT(var_hash);

    msgpack_serialize_zval(buf, val, &var_hash TSRMLS_CC);

    PHP_VAR_SERIALIZE_DESTROY(var_hash);
}

PHP_MSGPACK_API void php_msgpack_unserialize(
    zval *return_value, char *str, size_t str_len TSRMLS_DC)
{
    int ret;
    size_t off = 0;
    php_unserialize_data_t var_hash;

    if (str_len <= 0)
    {
        RETURN_NULL();
    }

    PHP_VAR_UNSERIALIZE_INIT(var_hash);

    ret = msgpack_unserialize_zval(
        &return_value, (const unsigned char *)str, str_len,
        &off, &var_hash TSRMLS_CC);

    switch (ret)
    {
        case MSGPACK_UNPACK_PARSE_ERROR:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) Parse error");
            RETVAL_NULL();
            break;
        case MSGPACK_UNPACK_CONTINUE:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) "
                       "Insufficient data for unserializeng");
            RETVAL_NULL();
            break;
        case MSGPACK_UNPACK_EXTRA_BYTES:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) Extra bytes");
        case MSGPACK_UNPACK_SUCCESS:
            break;
        default:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) Unknown result");
            RETVAL_NULL();
            break;
    }

    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

static PHP_FUNCTION(msgpack_serialize)
{
    zval *parameter;
    smart_str buf = {0};

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC, "z", &parameter) == FAILURE)
    {
        return;
    }

    php_msgpack_serialize(&buf, parameter TSRMLS_CC);

    ZVAL_STRINGL(return_value, buf.c, buf.len, 1);

    smart_str_free(&buf);
}

static PHP_FUNCTION(msgpack_unserialize)
{
    char *str;
    int str_len;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE)
    {
        return;
    }

    if (!str_len)
    {
        RETURN_NULL();
    }

    php_msgpack_unserialize(return_value, str, str_len TSRMLS_CC);
}
