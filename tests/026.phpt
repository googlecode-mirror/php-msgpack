--TEST--
Cyclic array test
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

$a = array(
    'a' => array(
        'b' => 'c',
        'd' => 'e'
    ),
);

$a['f'] = &$a;

test('array', $a, true);

$a = array("foo" => &$b);
$b = array(1, 2, $a);
var_dump($a);
var_dump($k = msgpack_unserialize(msgpack_serialize($a)));

$k["foo"][1] = "b";
var_dump($k);
?>
--EXPECT--
array
94a16194a162a163a164a165a16694a16194a162a163a164a165a16681a65f5f7265660005
OK
array(1) {
  ["foo"]=>
  &array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    array(1) {
      ["foo"]=>
      &array(3) {
        [0]=>
        int(1)
        [1]=>
        int(2)
        [2]=>
        *RECURSION*
      }
    }
  }
}
array(1) {
  ["foo"]=>
  &array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    array(1) {
      ["foo"]=>
      &array(3) {
        [0]=>
        int(1)
        [1]=>
        int(2)
        [2]=>
        array(1) {
          ["foo"]=>
          *RECURSION*
        }
      }
    }
  }
}
array(1) {
  ["foo"]=>
  &array(3) {
    [0]=>
    int(1)
    [1]=>
    string(1) "b"
    [2]=>
    array(1) {
      ["foo"]=>
      &array(3) {
        [0]=>
        int(1)
        [1]=>
        string(1) "b"
        [2]=>
        array(1) {
          ["foo"]=>
          *RECURSION*
        }
      }
    }
  }
}
