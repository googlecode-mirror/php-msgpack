--TEST--
Object test, __sleep and __wakeup exceptions
--SKIPIF--
<?php
if(!extension_loaded('msgpack')) {
    echo "skip no msgpack";
}
--FILE--
<?php

error_reporting(0);

function test($variable) {
    $serialized = msgpack_serialize($variable);
    $unserialized = msgpack_unserialize($serialized);
}

class Obj {
    private static $count = 0;
    var $a;
    var $b;

    function __construct($a, $b) {
        $this->a = $a;
        $this->b = $b;
    }

    function __sleep() {
        $c = self::$count++;
        if ($this->a) {
            throw new Exception("exception in __sleep $c");
        }
        return array('a', 'b');
    }

    function __wakeup() {
        $c = self::$count++;
        if ($this->b) {
            throw new Exception("exception in __wakeup $c");
        }
        $this->b = $this->a * 3;
    }
}


$a = new Obj(1, 0);
$b = new Obj(0, 1);
$c = new Obj(0, 0);

try {
    test($a);
} catch (Exception $e) {
    echo $e->getMessage(), PHP_EOL;
}

try {
    test($b);
} catch (Exception $e) {
    echo $e->getMessage(), PHP_EOL;
}

try {
    test($c);
} catch (Exception $e) {
    echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
exception in __sleep 0
exception in __wakeup 2
