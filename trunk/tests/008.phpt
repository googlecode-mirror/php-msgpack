--TEST--
Check for array+string serialization
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
    echo bin2hex($serialized),  PHP_EOL;
    echo $unserialized == $variable ? 'OK' : 'ERROR', PHP_EOL;
}

test('array("foo", "foo", "foo")', array("foo", "foo", "foo"));
test('array("one" => 1, "two" => 2))', array("one" => 1, "two" => 2));
test('array("kek" => "lol", "lol" => "kek")', array("kek" => "lol", "lol" => "kek"));
test('array("" => "empty")', array("" => "empty"));
?>
--EXPECT--
array("foo", "foo", "foo")
9600a3666f6f01a3666f6f02a3666f6f
OK
array("one" => 1, "two" => 2))
94a36f6e6501a374776f02
OK
array("kek" => "lol", "lol" => "kek")
94a36b656ba36c6f6ca36c6f6ca36b656b
OK
array("" => "empty")
92a0a5656d707479
OK
