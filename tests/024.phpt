--TEST--
Recursive objects
--SKIPIF--
--FILE--
<?php
if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

error_reporting(0);

function test($type, $variable, $test) {
    $serialized = msgpack_serialize($variable);
    $unserialized = msgpack_unserialize($serialized);

    echo $type, PHP_EOL;
    echo bin2hex($serialized), PHP_EOL;
    echo $test || $unserialized == $variable ? 'OK' : 'ERROR', PHP_EOL;
}

class Obj {
    public $a;
    protected $b;
    private $c;

    function __construct($a, $b, $c) {
        $this->a = $a;
        $this->b = $b;
        $this->c = $c;
    }
}

class Obj2 {
    public $aa;
    protected $bb;
    private $cc;
    private $obj;

    function __construct($a, $b, $c) {
        $this->a = $a;
        $this->b = $b;
        $this->c = $c;

        $this->obj = new Obj($a, $b, $c);
    }
}

class Obj3 {
    private $objs;

    function __construct($a, $b, $c) {
        $this->objs = array();

        for ($i = $a; $i < $c; $i += $b) {
            $this->objs[] = new Obj($a, $i, $c);
        }
    }
}

class Obj4 {
    private $a;
    private $obj;

    function __construct($a) {
        $this->a = $a;
    }

    public function set($obj) {
        $this->obj = $obj;
    }
}

$o2 = new Obj2(1, 2, 3);
test('objectrec', $o2, false);

$o3 = new Obj3(0, 1, 4);
test('objectrecarr', $o3, false);

$o4 = new Obj4(100);
$o4->set($o4);
test('objectselfrec', $o4, true);
?>
--EXPECT--
objectrec
88a85f5f636c61737300a44f626a32a26161c0a5002a006262c0a8004f626a32006363c0a9004f626a32006f626a84a85f5f636c61737300a34f626aa16101a4002a006202a6004f626a006303a16101a16202a16303
OK
objectrecarr
82a85f5f636c61737300a44f626a33aa004f626a33006f626a73980084a85f5f636c61737300a34f626aa16100a4002a006200a6004f626a0063040184a85f5f636c61737300a34f626aa16100a4002a006201a6004f626a0063040284a85f5f636c61737300a34f626aa16100a4002a006202a6004f626a0063040384a85f5f636c61737300a34f626aa16100a4002a006203a6004f626a006304
OK
objectselfrec
83a85f5f636c61737300a44f626a34a7004f626a34006164a9004f626a34006f626a81a65f5f7265660001
OK
