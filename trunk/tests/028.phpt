--TEST--
Serialize object into session, full set
--SKIPIF--
--FILE--
<?php

class Foo {
    private static $s1 = array();
    protected static $s2 = array();
    public static $s3 = array();

    private $d1;
    protected $d2;
    public $d3;

    public function __construct($foo) {
        $this->d1 = $foo;
        $this->d2 = $foo;
        $this->d3 = $foo;
    }
}

class Bar {
    private static $s1 = array();
    protected static $s2 = array();
    public static $s3 = array();

    public $d1;
    private $d2;
    protected $d3;

    public function __construct() {
    }

    public function set($foo) {
        $this->d1 = $foo;
        $this->d2 = $foo;
        $this->d3 = $foo;
    }
}

if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

$output = '';

function open($path, $name) {
    return true;
}

function close() {
    return true;
}

function read($id) {
    global $output;
    $output .= "read" . PHP_EOL;
    $a = new Bar();
    $b = new Foo($a);
    $a->set($b);
    $session = array('old' => $b);
    return msgpack_serialize($session);
}

function write($id, $data) {
    global $output;
    $output .= "write: ";
    $output .= bin2hex($data) . PHP_EOL;
    return true;
}

function destroy($id) {
    return true;
}

function gc($time) {
    return true;
}

ini_set('session.serialize_handler', 'msgpack');

session_set_save_handler('open', 'close', 'read', 'write', 'destroy', 'gc');

session_start();

$_SESSION['test'] = "foobar";
$a = new Bar();
$b = new Foo($a);
$a->set($b);
$_SESSION['new'] = $a;

session_write_close();

echo $output;
?>
--EXPECT--
read
write: 96a36f6c6484a85f5f636c61737300a3466f6fa700466f6f00643184a85f5f636c61737300a3426172a2643181a65f5f7265660001a70042617200643281a65f5f7265660001a5002a00643381a65f5f7265660001a5002a00643281a65f5f7265660002a2643381a65f5f7265660002a474657374a6666f6f626172a36e657784a85f5f636c61737300a3426172a2643184a85f5f636c61737300a3466f6fa700466f6f00643181a65f5f7265660004a5002a00643281a65f5f7265660004a2643381a65f5f7265660004a70042617200643281a65f5f7265660005a5002a00643381a65f5f7265660005
