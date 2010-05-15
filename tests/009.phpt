--TEST--
Check for reference serialisation
--INI--
report_memleaks=0
--SKIPIF--
<?php
if(!extension_loaded('msgpack')) {
    echo "skip no msgpack";
}
--FILE--
<?php

function test($type, $variable, $test) {
    $serialized = msgpack_serialize($variable);
    $unserialized = msgpack_unserialize($serialized);

    echo $type, PHP_EOL;
    echo bin2hex($serialized), PHP_EOL;
    echo $test || $unserialized == $variable ? 'OK' : 'ERROR', PHP_EOL;
}

$a = array('foo');

test('array($a, $a)', array($a, $a), false);
test('array(&$a, &$a)', array(&$a, &$a), false);

$a = array(null);
$b = array(&$a);
$a[0] = &$b;

test('cyclic', $a, true);

var_dump($a);
var_dump(msgpack_unserialize(msgpack_serialize($a)));

--EXPECT--
array($a, $a)
94009200a3666f6f019200a3666f6f
OK
array(&$a, &$a)
94009200a3666f6f0181a65f5f7265660002
OK
cyclic
92009200920081a65f5f7265660002
OK
array(1) {
  [0]=>
  &array(1) {
    [0]=>
    &array(1) {
      [0]=>
      &array(1) {
        [0]=>
        &array(1) {
          [0]=>
          *RECURSION*
        }
      }
    }
  }
}
array(1) {
  [0]=>
  &array(1) {
    [0]=>
    array(1) {
      [0]=>
      &array(1) {
        [0]=>
        array(1) {
          [0]=>
          *RECURSION*
        }
      }
    }
  }
}
