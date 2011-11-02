--TEST--
unpack: illegal key map
--FILE--
<?php
if(!extension_loaded('msgpack'))
{
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

error_reporting(0);

//JSON:{('foo1', 'foo2'): ('FOO3', 'FOO4'), ('hoge1', 'hoge2'): ('HOGE3', 'HOGE3')}
$binary = pack('H*', '8292a4666f6f31a4666f6f3292a4464f4f33a4464f4f3492a5686f676531a5686f67653292a5484f474533a5484f474533');


var_dump(msgpack_unpack($binary));


$msgpack = new MessagePack();
var_dump($msgpack->unpack($binary));
unset($msgpack);

$unpacker = new MessagePackUnpacker();
$unpacker->execute($binary);
var_dump($unpacker->data());
unset($unpacker);



ini_set('msgpack.illegal_key_insert', 1);

var_dump(msgpack_unpack($binary));


$msgpack = new MessagePack();
var_dump($msgpack->unpack($binary));
unset($msgpack);

$unpacker = new MessagePackUnpacker();
$unpacker->execute($binary);
var_dump($unpacker->data());
unset($unpacker);

--EXPECTF--
array(1) {
  ["Array"]=>
  array(2) {
    [0]=>
    string(5) "HOGE3"
    [1]=>
    string(5) "HOGE3"
  }
}
array(1) {
  ["Array"]=>
  array(2) {
    [0]=>
    string(5) "HOGE3"
    [1]=>
    string(5) "HOGE3"
  }
}
array(1) {
  ["Array"]=>
  array(2) {
    [0]=>
    string(5) "HOGE3"
    [1]=>
    string(5) "HOGE3"
  }
}
array(4) {
  [0]=>
  array(2) {
    [0]=>
    string(4) "foo1"
    [1]=>
    string(4) "foo2"
  }
  [1]=>
  array(2) {
    [0]=>
    string(4) "FOO3"
    [1]=>
    string(4) "FOO4"
  }
  [2]=>
  array(2) {
    [0]=>
    string(5) "hoge1"
    [1]=>
    string(5) "hoge2"
  }
  [3]=>
  array(2) {
    [0]=>
    string(5) "HOGE3"
    [1]=>
    string(5) "HOGE3"
  }
}
array(4) {
  [0]=>
  array(2) {
    [0]=>
    string(4) "foo1"
    [1]=>
    string(4) "foo2"
  }
  [1]=>
  array(2) {
    [0]=>
    string(4) "FOO3"
    [1]=>
    string(4) "FOO4"
  }
  [2]=>
  array(2) {
    [0]=>
    string(5) "hoge1"
    [1]=>
    string(5) "hoge2"
  }
  [3]=>
  array(2) {
    [0]=>
    string(5) "HOGE3"
    [1]=>
    string(5) "HOGE3"
  }
}
array(4) {
  [0]=>
  array(2) {
    [0]=>
    string(4) "foo1"
    [1]=>
    string(4) "foo2"
  }
  [1]=>
  array(2) {
    [0]=>
    string(4) "FOO3"
    [1]=>
    string(4) "FOO4"
  }
  [2]=>
  array(2) {
    [0]=>
    string(5) "hoge1"
    [1]=>
    string(5) "hoge2"
  }
  [3]=>
  array(2) {
    [0]=>
    string(5) "HOGE3"
    [1]=>
    string(5) "HOGE3"
  }
}
