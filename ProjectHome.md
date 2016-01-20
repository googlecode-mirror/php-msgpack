# Summary #
This extension provide API for communicating with <a href='http://msgpack.sourceforge.net/'>MessagePack</a> serialization.

Development code will be.

The official version of the code has been integrated into the MessagePack project. (<a href='http://github.com/msgpack/msgpack/tree/master/php/'>here</a>)

# Installation #
## pecl command ##
```
$ pecl channel-discover php-msgpack.googlecode.com/svn/pecl
$ pecl install msgpack/msgpack-beta
```

After enabling the extension in php.ini, restarting the web service is required for the changes to be picked up.

## building ##
```
$ phpize
$ ./configure
$ make
# make install
```

A successful install will have created msgpack.so and put it into the PHP extensions directory.
You'll need to and adjust php.ini and add an extension=msgpack.so line before you can use the extension.

For the Windows binaries and installer, [see](http://code.google.com/p/php-msgpack/wiki/Windows)

# Features #
  * Supports same data types as the standard PHP serializer
    * Booleans
    * Integers
    * Floating point numbers
    * Strings
    * Arrays
    * Objects
    * NULL
  * ＿autoload
  * unserialize\_callback\_func
  * ＿sleep and ＿wakeup
  * Serializable interface
  * Stream deserializer

# Function #
msgpack\_serialize — Generates a storable representation of a value
```
string msgpack_serialize ( mixed $value )
```

msgpack\_unserialize —  Creates a PHP value from a stored representation
```
mixed msgpack_unserialize ( string $str [, mixed $object ] )
```

# Classes #
  * MessagePack
  * MessagePackUnpacker

# Example #
```
<?php
// serialize
$value = 'example';
$binary = msgpack_serialize($value);

// unserialize
$value = msgpack_unserialize($binary);
```

## Session ##
```
<?php
// session handler
ini_set('session.serialize_handler', 'msgpack');
```