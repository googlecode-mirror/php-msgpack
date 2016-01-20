Can be converted to class objects during unserializing.
(Version 0.4.0 and later will be implemented)

## Examples ##
```
$data = array('a', 'b', 'c');

class Obj
{
    public $a;
    protected $b;
    private $c;

    public function __construct($a = null, $b = null, $c = null)
    {
        $this->a = $a;
        $this->b = $b;
        $this->c = $c;
    }
}

$serialize = msgpack_serialize($data);
/*
array(3) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  string(1) "c"
}
*/

$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  string(1) "a"
  ["b":protected]=>
  string(1) "b"
  ["c":"Obj":private]=>
  string(1) "c"
}
*/
```

# Specification #

## Argument ##

  * Class name or class object is appointed to the second argument of msgpack\_unserialize function.

```
$unserialize = msgpack_unserialize($serialize, 'Obj');
$unserialize = msgpack_unserialize($serialize, new Obj());
```

  * Undefined class name and other than of class object is appointed, 'Fatal error' occurs.

```
$unserialize = msgpack_unserialize($serialize, 'Foo');
/*
Fatal error: [msgpack] (msgpack_convert_object) Class 'Test' not found
*/

$unserialize = msgpack_unserialize($serialize, null);
$unserialize = msgpack_unserialize($serialize, 123);
$unserialize = msgpack_unserialize($serialize, 123.45);
$unserialize = msgpack_unserialize($serialize, true);
$unserialize = msgpack_unserialize($serialize, array());
/*
Fatal error: [msgpack] (msgpack_convert_object) Object type is unsupported
*/
```

### MessagePack ###

  * Class name or class object is appointed to the second argument of MessagePack::unpack method.

```
$msgpack = new MessagePack();

$unserialize = $msgpack->unpack($serialize, 'Obj');
$unserialize = $msgpack->unpack($serialize, new Obj());
```

### MessagePackUnpacker ###

  * Class name or class object is appointed to argument of MessagePackUnpacker::data method.

```
$unpacker = new MessagePackUnpacker();
$unpacker->execute($serialize);

$unserialize = $unpacker->data('Obj');
$unserialize = $unpacker->data(new Obj());
```


## Convert ##

  * It is set to the class property.

  * In case of array it is set to the defined order of the property.

```
$data = array('a', 'b', 'c');
$serialize = msgpack_serialize($data);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  string(1) "a"
  ["b":protected]=>
  string(1) "b"
  ["c":"Obj":private]=>
  string(1) "c"
}
*/
```

  * In case of association array it is set to the property of associative key name.

```
$data = array('c' => 'C', 'b' => 'b', 'a' => 'A');
$serialize = msgpack_serialize($data);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  string(1) "A"
  ["b":protected]=>
  string(1) "B"
  ["c":"Obj":private]=>
  string(1) "C"
}
*/
```

  * Priority is given to the thing of a string key.

```
$data = array('c' => 'C', 'a' => 'A', 'b');
$serialize = msgpack_serialize($data);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  string(1) "A"
  ["b":protected]=>
  string(1) "b"
  ["c":"Obj":private]=>
  string(1) "C"
}
*/
```

  * In case of the same class object of a conversion place does not have conversion processing performed.

```
class Obj
{
    public $a;
    protected $b;
    private $c;

    public function __construct($a = null, $b = null, $c = null)
    {
        $this->a = $a;
        $this->b = $b;
        $this->c = $c;
    }
}

$data = new Obj('a', 'b', 'c');
$serialize = msgpack_serialize($data);

$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  string(1) "a"
  ["b":protected]=>
  string(1) "b"
  ["c":"Obj":private]=>
  string(1) "c"
}
*/

class Test
{
    public $a;
    protected $b;
    private $c;
}

$unserialize = msgpack_unserialize($serialize, 'Test');
/*
object(Test)#1 (3) {
  ["a"]=>
  object(Obj)#1 (3) {
    ["a"]=>
    string(1) "a"
    ["b":protected]=>
    string(1) "b"
    ["c":"Obj":private]=>
    string(1) "c"
  }
  ["b":protected]=>
  NULL
  ["c":"Obj":private]=>
  NULL
}
*/
```

  * It becomes the property of the numerical key in case of the class which does not have the property.
    * Notice: As the class property it cannot access with this state.
    * Notice: Casting process is the same as an array of objects ((object)$data).

```
$unserialize = msgpack_unserialize($serialize, 'stdClass');
/*
object(stdClass)#1 (3) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  string(1) "c"
}
*/
```

  * In case of scalar value it is set to the first property.

```
$serialize = msgpack_serialize(null);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  NULL
  ["b":protected]=>
  NULL
  ["c":"Obj":private]=>
  NULL
}
*/

$serialize = msgpack_serialize(123);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  123
  ["b":protected]=>
  NULL
  ["c":"Obj":private]=>
  NULL
}
*/

$serialize = msgpack_serialize(123.45);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  123.45
  ["b":protected]=>
  NULL
  ["c":"Obj":private]=>
  NULL
}
*/

$serialize = msgpack_serialize(true);
$unserialize = msgpack_unserialize($serialize, 'Obj');
/*
object(Obj)#1 (3) {
  ["a"]=>
  true
  ["b":protected]=>
  NULL
  ["c":"Obj":private]=>
  NULL
}
*/
```