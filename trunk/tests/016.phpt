--TEST--
Object test, __sleep
--SKIPIF--
--FILE--
<?php
if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

function test($type, $variable, $test) {
    $serialized = msgpack_serialize($variable);
    $unserialized = msgpack_unserialize($serialized);

    echo $type, PHP_EOL;
    echo bin2hex($serialized), PHP_EOL;
    var_dump($unserialized);
    echo $test || $unserialized == $variable ? 'OK' : 'ERROR', PHP_EOL;
}

class Obj {
    public $a;
    protected $b;
    private $c;
    var $d;

    function __construct($a, $b, $c, $d) {
        $this->a = $a;
        $this->b = $b;
        $this->c = $c;
        $this->d = $d;
    }

    function __sleep() {
        return array('a', 'b', 'c');
    }

#   function __wakeup() {
#       $this->d = $this->a + $this->b + $this->c;
#   }
}

$o = new Obj(1, 2, 3, 4);


test('object', $o, true);
?>
--EXPECTF--
object
84c0a34f626aa16101a4002a006202a6004f626a006303
object(Obj)#%d (4) {
  ["a"]=>
  int(1)
  ["b":protected]=>
  int(2)
  ["c":"Obj":private]=>
  int(3)
  ["d"]=>
  NULL
}
OK
