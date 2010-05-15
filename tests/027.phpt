--TEST--
Check for serialization handler
--SKIPIF--
<?php
echo "skip";
?>
--FILE--
<?php
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
    return pack('H*', '0000000214011103666f6f0601');
}

function write($id, $data) {
    global $output;
    $output .= "wrote: ";
    $output .= substr(bin2hex($data), 8). PHP_EOL;
    return true;
}

function destroy($id) {
    return true;
}

function gc($time) {
    return true;
}

class Foo {
}

class Bar {
}

ini_set('session.serialize_handler', 'msgpack');

session_set_save_handler('open', 'close', 'read', 'write', 'destroy', 'gc');


$db_object = new Foo();
$session_object = new Bar();

$v = session_start();
var_dump($v);
$_SESSION['test'] = "foobar";

session_write_close();

echo $output;
?>
--EXPECT--
bool(true)
read
wrote: 14021103666f6f06011104746573741106666f6f626172
