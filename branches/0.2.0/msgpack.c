
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

/*
 * TODO:
 * - stream serializer : class - MessagePackUnpacker : [ OK ]
 * - alias function : msgpack_pack, msgpack_unpack : [ OK ]
 * - class object : class - MessagePack : pack / unpack / unpacker
 */

static ZEND_FUNCTION(msgpack_serialize);
static ZEND_FUNCTION(msgpack_unserialize);

static ZEND_METHOD(msgpack, __construct);
static ZEND_METHOD(msgpack, __destruct);
static ZEND_METHOD(msgpack, feed);
static ZEND_METHOD(msgpack, execute);
static ZEND_METHOD(msgpack, data);
static ZEND_METHOD(msgpack, reset);

#if ZEND_MODULE_API_NO >= 20060613
#define MAGPACK_METHOD_BASE(classname, name) zim_##classname##_##name
#else
#define MSGPACK_METHOD_BASE(classname, name) zif_##classname##_##name
#endif

#define MSGPACK_METHOD(classname, name, retval, thisptr) \
    MAGPACK_METHOD_BASE(classname, name)(0, retval, NULL, thisptr, 0 TSRMLS_CC)

#define MSGPACK_UNPACKER_OBJECT       \
    php_msgpack_unpacker_t *unpacker; \
    unpacker =(php_msgpack_unpacker_t *)zend_object_store_get_object(getThis() TSRMLS_CC);

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_serialize, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_unserialize, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_class___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_class___destruct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_class_feed, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_class_execute, 1, 0, 0)
    ZEND_ARG_INFO(0, str)
    ZEND_ARG_INFO(1, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_class_data, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_class_reset, 0, 0, 0)
ZEND_END_ARG_INFO()

PS_SERIALIZER_FUNCS(msgpack);

static const zend_function_entry msgpack_functions[] = {
    ZEND_FE(msgpack_serialize, arginfo_msgpack_serialize)
    ZEND_FE(msgpack_unserialize, arginfo_msgpack_unserialize)
    ZEND_FALIAS(msgpack_pack, msgpack_serialize, arginfo_msgpack_serialize)
    ZEND_FALIAS(msgpack_unpack, msgpack_unserialize,
                arginfo_msgpack_unserialize)
    {NULL, NULL, NULL}
};

static const zend_function_entry msgpack_unpacker_methods[] = {
    ZEND_ME(msgpack, __construct,
            arginfo_msgpack_class___construct, ZEND_ACC_PUBLIC)
    ZEND_ME(msgpack, __destruct,
            arginfo_msgpack_class___destruct, ZEND_ACC_PUBLIC)
    ZEND_ME(msgpack, feed, arginfo_msgpack_class_feed, ZEND_ACC_PUBLIC)
    ZEND_ME(msgpack, execute, arginfo_msgpack_class_execute, ZEND_ACC_PUBLIC)
    ZEND_ME(msgpack, data, arginfo_msgpack_class_data, ZEND_ACC_PUBLIC)
    ZEND_ME(msgpack, reset, arginfo_msgpack_class_reset, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

static zend_class_entry *msgpack_ce = NULL;

typedef struct {
    zend_object object;
    smart_str buffer;
    zval *retval;
    long offset;
} php_msgpack_unpacker_t;

static void php_msgpack_unpacker_free(
    php_msgpack_unpacker_t *unpacker TSRMLS_DC)
{
    zend_object_std_dtor(&unpacker->object TSRMLS_CC);
    efree(unpacker);
}

static zend_object_value php_msgpack_unpacker_new(
    zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    zval *tmp;
    php_msgpack_unpacker_t *unpacker;

    unpacker = emalloc(sizeof(php_msgpack_unpacker_t));

    zend_object_std_init(&unpacker->object, ce TSRMLS_CC);

    zend_hash_copy(
        unpacker->object.properties, &ce->default_properties,
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

    retval.handle = zend_objects_store_put(
        unpacker, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_msgpack_unpacker_free,
        NULL TSRMLS_CC);
    retval.handlers = zend_get_std_object_handlers();

    return retval;
}

static ZEND_MINIT_FUNCTION(msgpack)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "MessagePackUnpacker", msgpack_unpacker_methods);
    msgpack_ce = zend_register_internal_class(&ce TSRMLS_CC);
    msgpack_ce->create_object = php_msgpack_unpacker_new;

    MSGPACK_G(error_display) = 1;

#if HAVE_PHP_SESSION
    php_session_register_serializer("msgpack",
                                    PS_SERIALIZER_ENCODE_NAME(msgpack),
                                    PS_SERIALIZER_DECODE_NAME(msgpack));
#endif

    return SUCCESS;
}

static ZEND_MINFO_FUNCTION(msgpack)
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
    ZEND_MINIT(msgpack),
    NULL,
    NULL,
    NULL,
    ZEND_MINFO(msgpack),
#if ZEND_MODULE_API_NO >= 20010901
    MSGPACK_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_MSGPACK
ZEND_GET_MODULE(msgpack)
#endif

PS_SERIALIZER_ENCODE_FUNC(msgpack)
{
    smart_str buf = {0};
    php_serialize_data_t var_hash;

    PHP_VAR_SERIALIZE_INIT(var_hash);

    msgpack_serialize_zval(&buf, PS(http_session_vars), &var_hash TSRMLS_CC);

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
    HashTable *tmp_hash;
    HashPosition tmp_hash_pos;
    char *key_str;
    ulong key_long;
    uint key_len;
    zval *tmp;
    zval **data;
    msgpack_unserialize_data mpsd;

    PHP_VAR_UNSERIALIZE_INIT(var_hash);

    MAKE_STD_ZVAL(tmp);

    mpsd.data = (unsigned char *)val;;
    mpsd.length = vallen;
    mpsd.offset = 0;

    ret = msgpack_unserialize_zval(&tmp, &mpsd, &var_hash TSRMLS_CC);

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
    php_unserialize_data_t var_hash;
    msgpack_unserialize_data mpsd;

    if (str_len <= 0)
    {
        RETURN_NULL();
    }

    PHP_VAR_UNSERIALIZE_INIT(var_hash);

    mpsd.data = (unsigned char *)str;
    mpsd.length = str_len;
    mpsd.offset = 0;

    ret = msgpack_unserialize_zval(&return_value, &mpsd, &var_hash TSRMLS_CC);

    switch (ret)
    {
        case MSGPACK_UNPACK_PARSE_ERROR:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) Parse error");
            break;
        case MSGPACK_UNPACK_CONTINUE:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) "
                       "Insufficient data for unserializeng");
            break;
        case MSGPACK_UNPACK_EXTRA_BYTES:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) Extra bytes");
            break;
        case MSGPACK_UNPACK_SUCCESS:
            break;
        default:
            zend_error(E_WARNING,
                       "[msgpack] (php_msgpack_unserialize) Unknown result");
            break;
    }

    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

static ZEND_FUNCTION(msgpack_serialize)
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

static ZEND_FUNCTION(msgpack_unserialize)
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


static ZEND_METHOD(msgpack, __construct)
{
    MSGPACK_UNPACKER_OBJECT;

    unpacker->buffer.c = NULL;
    unpacker->buffer.len = 0;
    unpacker->buffer.a = 0;
    unpacker->retval = NULL;
    unpacker->offset = 0;
}

static ZEND_METHOD(msgpack, __destruct)
{
    MSGPACK_METHOD(msgpack, reset, NULL, getThis());
}

static ZEND_METHOD(msgpack, feed)
{
    char *str;
    int str_len;
    MSGPACK_UNPACKER_OBJECT;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE)
    {
        return;
    }

    if (!str_len)
    {
        RETURN_FALSE;
    }

    smart_str_appendl(&unpacker->buffer, str, str_len);

    RETURN_TRUE;
}

static ZEND_METHOD(msgpack, execute)
{
    char *str = NULL;
    long str_len = 0;
    zval *offset;
    int ret;
    php_unserialize_data_t var_hash;
    msgpack_unserialize_data mpsd;
    MSGPACK_UNPACKER_OBJECT;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS() TSRMLS_CC, "|sz/",
            &str, &str_len, &offset) == FAILURE)
    {
        return;
    }

    if (str != NULL)
    {
        mpsd.data = (unsigned char *)str;
        mpsd.length = str_len;
        mpsd.offset = Z_LVAL_P(offset);
    }
    else
    {
        mpsd.data = (unsigned char *)unpacker->buffer.c;
        mpsd.length = unpacker->buffer.len;
        mpsd.offset = unpacker->offset;
    }

    if (mpsd.length <= 0 || mpsd.length == mpsd.offset)
    {
        RETURN_FALSE;
    }

    PHP_VAR_UNSERIALIZE_INIT(var_hash);

    if (unpacker->retval == NULL)
    {
        ALLOC_INIT_ZVAL(unpacker->retval);
    }

    MSGPACK_G(error_display) = 0;

    ret = msgpack_unserialize_zval(
        &unpacker->retval, &mpsd, &var_hash TSRMLS_CC);

    MSGPACK_G(error_display) = 1;

    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

    if (str != NULL)
    {
        ZVAL_LONG(offset, mpsd.offset);
    }
    else
    {
        unpacker->offset = mpsd.offset;
    }

    switch (ret)
    {
        case MSGPACK_UNPACK_SUCCESS:
            RETURN_TRUE;
        default:
            RETURN_FALSE;
    }
}

static ZEND_METHOD(msgpack, data)
{
    MSGPACK_UNPACKER_OBJECT;

    RETURN_ZVAL(unpacker->retval, 1, 1);
}

static ZEND_METHOD(msgpack, reset)
{
    MSGPACK_UNPACKER_OBJECT;

    smart_str_free(&unpacker->buffer);

    unpacker->buffer.c = NULL;
    unpacker->buffer.len = 0;
    unpacker->buffer.a = 0;
    unpacker->offset = 0;

    if (unpacker->retval != NULL)
    {
        zval_ptr_dtor(&unpacker->retval);
        unpacker->retval = NULL;
    }
}
