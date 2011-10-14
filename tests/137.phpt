--TEST--
Bug array pack/unpack
--FILE--
<?php
if(!extension_loaded('msgpack'))
{
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

$data = array(
    "flags" => array(
        0 => 16777216,
        2 => 4
    )
);

$data['flags'][1] = 65536;

var_dump(msgpack_unpack(msgpack_pack($data)) == $data);

$data = array(
    1 => array(
        0 => 16777216,
        2 => 4
    )
);

$data[0][0] = 137;
$data[1][1] = 65536;

var_dump(msgpack_unpack(msgpack_pack($data)) == $data);

--EXPECTF--
bool(true)
bool(true)
