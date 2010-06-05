--TEST--
Unserialize invalid data
--SKIPIF--
--FILE--
<?php
if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

$datas = array(
    87817,
    -1,
    array(1,2,3,"testing" => 10, "foo"),
    true,
    false,
    0.187182,
    "dakjdh98389\000",
    null,
    (object)array(1,2,3),
);

error_reporting(0);

foreach ($datas as $data) {
    $str = msgpack_serialize($data);
    $len = strlen($str);

    // truncated
    for ($i = 0; $i < $len - 1; $i++) {
        $v = msgpack_unserialize(substr($str, 0, $i));

        if (is_object($data) || is_array($data)) {
            if ($v !== null && $v != $data) {
                echo "output at $i:\n";
                var_dump($v);
            }
        } else if ($v !== null && $v == $data) {
            continue;
        } else if ($v !== null && $v !== $data) {
            echo "output at $i:\n";
            var_dump($v);
            echo "vs.\n";
            var_dump($data);
        }
    }

    // padded
    $str .= "98398afa\000y21_ ";
    $v = msgpack_unserialize($str);
    if ($v !== $data && !(is_object($data) && $v == $data)) {
        echo "padded should get original\n";
        var_dump($v);
        echo "vs.\n";
        var_dump($data);
    }
}
?>
--EXPECTF--
output at 18:
array(5) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  ["testing"]=>
  int(10)
  [3]=>
  NULL
}
output at 19:
array(5) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  ["testing"]=>
  int(10)
  [3]=>
  NULL
}
output at 9:
object(__PHP_Incomplete_Class)#%d (4) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(0) ""
  [115]=>
  int(116)
  [100]=>
  int(67)
  [108]=>
  int(97)
}
output at 10:
object(__PHP_Incomplete_Class)#%d (4) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(0) ""
  [115]=>
  int(116)
  [100]=>
  int(67)
  [108]=>
  int(97)
}
