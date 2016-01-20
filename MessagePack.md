# The MessagePack class methods #

## MessagePack::construct ##
Creates a new MessagePack object

```
MessagePack::__construct ([ bool $value ]) 
```

### Parameters ###
| value | _true_ | runtime MessagePack for PHP Only |
|:------|:-------|:---------------------------------|
| value | _false_ | runtime MessagePack for standard ( classes are serialized as an array) |

If no parameters are passed, specified in php.ini for msgpack.php\_only.
This is enabled by default.

### Return Values ###
Returns a new MessagePack object.


## MessagePack::setOption ##
Set a MessagePack option.

```
public bool MessagePack::setOption ( int $option, mixed $value )
```

### Parameters ###
| option | _MessagePack::OPT\_PHPONLY_ |
|:-------|:----------------------------|
| value  | _true_                      | runtime MessagePack for PHP Only |
| value  | _false_                     | runtime MessagePack for standard ( classes are serialized as an array) |

### Return Values ###
Returns TRUE if the option could be set; FALSE if not.


## MessagePack::pack ##
Generates a storable representation of a value.

```
public string MessagePack::pack ( mixed $value ) 
```

### Parameters ###
| value | The value to be serialized |
|:------|:---------------------------|

### Return Values ###
Returns a string containing a byte-stream representation of value that can be stored anywhere.


## MessagePack::unpack ##
Creates a PHP value from a stored representation.

```
public mixed MessagePack::unpack ( string $str [, mixed $object ] )
```

### Parameters ###
| str | The serialized string |
|:----|:----------------------|
| object | Class name or object  |

### Return Values ###
The converted value is returned, and can be a boolean, integer, float, string, array or object.


## MessagePack::unpacker ##
Gets a MessagePackUnpacker object.

```
public MessagePackUnpacker MessagePack::unpacker ([ bool $value ])
```

### Parameters ###
| value | _true_ | runtime MessagePack for PHP Only |
|:------|:-------|:---------------------------------|
| value | _false_ | runtime MessagePack for standard ( classes are serialized as an array) |

If no parameters are passed, specified in MessagePack options.

### Return Values ###
Returns the MessagePackUnpacker object.

## Examples ##
```
$value = 'test';

//gets instance
$msgpack = new MessagePack();

//pack
$serialized = $msgpack->pack($value);

//unpack
$retval = $msgpack->unpack($serialized);

//unpacker
$unpacker = $msgpack->unpacker();
```