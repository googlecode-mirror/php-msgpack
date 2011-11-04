--TEST--
pack: empty array/map
--FILE--
<?php
if(!extension_loaded('msgpack'))
{
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

function test($value)
{
    var_dump(bin2hex(msgpack_pack($value)));


    $msgpack = new MessagePack();
    var_dump(bin2hex($msgpack->pack($value)));
    unset($msgpack);

    $msgpack = new MessagePack();
    $msgpack->setOption(MessagePack::OPT_PHPONLY, false);
    var_dump(bin2hex($msgpack->pack($value)));
    unset($msgpack);

    $msgpack = new MessagePack(false);
    var_dump(bin2hex($msgpack->pack($value)));
    unset($msgpack);
}


test(new stdClass());
test(array());

--EXPECTF--
string(2) "80"
string(2) "80"
string(2) "80"
string(2) "80"
string(2) "90"
string(2) "90"
string(2) "90"
string(2) "90"
