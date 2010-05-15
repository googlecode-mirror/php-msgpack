--TEST--
Check for simple array serialization
--SKIPIF--
--FILE--
<?php
if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

function test($type, $variable) {
    $serialized = msgpack_serialize($variable);
    $unserialized = msgpack_unserialize($serialized);

    echo $type, PHP_EOL;
    echo bin2hex($serialized), PHP_EOL;
    echo $unserialized == $variable ? 'OK' : 'ERROR', PHP_EOL;
}

test('empty array:', array());
test('array(1, 2, 3)', array(1, 2, 3));
test('array(array(1, 2, 3), arr...', array(array(1, 2, 3), array(4, 5, 6), array(7, 8, 9)));
?>
--EXPECT--
empty array:
90
OK
array(1, 2, 3)
96000101020203
OK
array(array(1, 2, 3), arr...
96009600010102020301960004010502060296000701080209
OK
