--TEST--
Check for serialization handler, ini-directive
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
    return pack('H*', '0000000214011103666f6f0601');
}

function write($id, $data) {
    global $output;
    $output .= bin2hex($data) . PHP_EOL;
    return true;
}

function destroy($id) {
    return true;
}

function gc($time) {
    return true;
}

session_set_save_handler('open', 'close', 'read', 'write', 'destroy', 'gc');

session_start();

echo ++$_SESSION['foo'], PHP_EOL;

session_write_close();

echo $output;
?>
--EXPECT--
2
14011103666f6f0602
