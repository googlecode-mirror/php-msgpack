--TEST--
unpack: empty array/map
--FILE--
<?php
if(!extension_loaded('msgpack'))
{
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

function test($serialize)
{
    var_dump(msgpack_unpack($serialize));


    $msgpack = new MessagePack();
    var_dump($msgpack->unpack($serialize));
    unset($msgpack);

    $msgpack = new MessagePack();
    $msgpack->setOption(MessagePack::OPT_PHPONLY, false);
    var_dump($msgpack->unpack($serialize));

    $msgpack = new MessagePack(false);
    var_dump($msgpack->unpack($serialize));

    $unpacker = new MessagePackUnpacker();
    $unpacker->execute($serialize);
    var_dump($unpacker->data());

    $unpacker->reset();

    $unpacker->feed($serialize);
    $unpacker->execute();
    var_dump($unpacker->data());
}

test(pack('H*', '80'));
test(pack('H*', '90'));

--EXPECTF--
object(stdClass)#%d (0) {
}
object(stdClass)#%d (0) {
}
array(0) {
}
array(0) {
}
object(stdClass)#%d (0) {
}
object(stdClass)#%d (0) {
}
array(0) {
}
array(0) {
}
array(0) {
}
array(0) {
}
array(0) {
}
array(0) {
}
