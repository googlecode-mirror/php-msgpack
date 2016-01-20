# The MessagePackUnpacker class methods #

## MessagePackUnpacker::construct ##
Creates a new MessagePackUnpacker object

```
MessagePackUnpacker::__construct ([ bool $value ]) 
```

### Parameters ###
| value | _true_ | runtime MessagePack for PHP Only |
|:------|:-------|:---------------------------------|
| value | _false_ | runtime MessagePack for standard |

If no parameters are passed, specified in php.ini for msgpack.php\_only.
This is enabled by default.

### Return Values ###
Returns a new MessagePackUnpacker object.


## MessagePackUnpacker::setOption ##
Set a MessagePackUnpacker option.

```
public bool MessagePackUnpacker::setOption ( int $option, mixed $value )
```

### Parameters ###
| option | _MessagePack::OPT\_PHPONLY_ |
|:-------|:----------------------------|
| value  | _true_                      | runtime MessagePack for PHP Only |
| value  | _false_                     | runtime MessagePack for standard |

### Return Values ###
Returns TRUE if the option could be set; FALSE if not.


## MessagePackUnpacker::feed ##
Fills the internal buffer with the specified buffer.

```
public boolean MessagePackUnpacker::feed ( string $str )
```

### Parameters ###
| str | The serialized string |
|:----|:----------------------|

### Return Values ###
If the buffer was successful.


## MessagePackUnpacker::execute ##
Deserializes one object over the specified internal buffer or str from offset bytes.

```
public boolean MessagePackUnpacker::execute ( [ string $str, int &$offset ] )
```

### Parameters ###
| str          | The serialized string |
|:-------------|:----------------------|
| offset (in)  | start of removed portion is at that offset from the beginning of the serialized string |
| offset (out) | reading bytes of serialized string |

### Return Values ###
Returns true if an object is ready to get with data method.


## MessagePackUnpacker::data ##
Gets the object deserialized by execute method.

```
public mixed MessagePackUnpacker::data ( [mixed $object ] )
```

### Parameters ###
| object | Class naem or object |
|:-------|:---------------------|

### Return Values ###
The converted value is returned, and can be a boolean, integer, float, string, array or object.


## MessagePackUnpacker::reset ##
Resets the internal state of the unpacker.

```
public void MessagePackUnpacker::reset ( void )
```


## Examples ##
```
$value = 'test';

//gets instance
$unpacker = new MessagePackUnpacker();
//$msgpack = new MessagePack();
//$unpacker = $msgpack->unpacker();

//pack
$serialized = $msgpack->pack($value);
$length = strlen($serialized);


//buffered streaming unpack
for ($i = 0; $i < $length;) {
    $read = rand(1, 10);
    $str = substr($serialized, $i, $read);

    $unpacker->feed($str);
    while ($unpacker->execute()) {
        $unserialized = $unpacker->data();
        var_dump($unserialized);
        $unpacker->reset();
    }

    $i += $read;
}

//unbuffered streaming unpack
$str = "";
$offset = 0;

for ($i = 0; $i < $length;) {
    $read = rand(1, 10);
    $str = $str . substr($serialized, $i, $read);

    while ($unpacker->execute($str, $offset)) {
        $unserialized = $unpacker->data();
        var_dump($unserialized);
        $unpacker->reset();
        $str = substr($str, $offset);
        $offset = 0;
    }

    $i += $read;
}
```