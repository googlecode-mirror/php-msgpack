# Classes #

Instances of this class are used to interact with a serializer.

  * MessagePack
  * MessagePackUnpacker

# Synopsis #
## MessagePack ##
```
MessagePack   {
    /* Constants */
    const MessagePack::OPT_PHPONLY;

    /* Methods */
    __construct  ([ bool $value ])
    public bool setOption ( int $option, mixed $value )
    public string pack ( mixed $value )
    public mixed unpack ( string $str )
    public MessagePackUnpacker unpacker ([ bool $value ])
}
```

### Warning ###
MessagePack uses class constants since PHP 5.1.
Prior releases use global constants in the form MESSAGEPACK\_OPT\_PHPONLY.

## MessagePackUnpacker ##
```
MessagePackUnpacker {
    /* Methods */
    __construct  ([ bool $value ])
    public bool setOption ( int $option, mixed $value )
    public boolean feed ( string $str )
    public boolean execute ( [ string $str, int $offset ] )
    public mixed data ( void )
    public void reset ( void )
}
```