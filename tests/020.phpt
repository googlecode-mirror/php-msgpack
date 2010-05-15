--TEST--
Object test, incomplete class
--SKIPIF--
--FILE--
<?php
if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

function test($type, $variable, $test) {
    $serialized = pack('H*', $variable);
    $unserialized = msgpack_unserialize($serialized);

    echo $type, PHP_EOL;
    echo bin2hex($serialized), PHP_EOL;
    var_dump($unserialized);
}

test('incom', '83a85f5f636c61737300a34f626aa16101a16202', false);

/*
 * you can add regression tests for your extension here
 *
 * the output of your test code has to be equal to the
 * text in the --EXPECT-- section below for the tests
 * to pass, differences between the output and the
 * expected text are interpreted as failure
 *
 * see php5/README.TESTING for further information on
 * writing regression tests
 */
?>
--EXPECTF--
incom
83a85f5f636c61737300a34f626aa16101a16202
object(__PHP_Incomplete_Class)#%d (3) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(3) "Obj"
  ["a"]=>
  int(1)
  ["b"]=>
  int(2)
}
