
#ifndef MSGPACL_PACK_H
#define MSGPACL_PACK_H

#include "ext/standard/php_smart_str.h"

#include "msgpack/pack_define.h"

#define msgpack_pack_inline_func(name) \
    static inline void msgpack_pack ## name

#define msgpack_pack_inline_func_cint(name) \
    static inline void msgpack_pack ## name

#define msgpack_pack_user smart_str*

#define msgpack_pack_append_buffer(user, buf, len) \
    smart_str_appendl(user, (const void*)buf, len)

#include "msgpack/pack_template.h"

#endif
