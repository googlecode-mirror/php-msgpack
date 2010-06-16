--TEST--
Serialize object into session, full set
--SKIPIF--
--FILE--
<?php
if(!extension_loaded('msgpack')) {
    dl('msgpack.' . PHP_SHLIB_SUFFIX);
}

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
var_dump($_SESSION);
?>
--EXPECTF--
read
write: 83a36f6c6484c0a3466f6fa700466f6f00643184c0a3426172a2643182c002c002a70042617200643282c002c002a5002a00643382c002c002a5002a00643282c002c003a2643382c002c003a474657374a6666f6f626172a36e657784c0a3426172a2643184c0a3466f6fa700466f6f00643182c002c00aa5002a00643282c002c00aa2643382c002c00aa70042617200643282c002c00ba5002a00643382c002c00b
array(3) {
  ["old"]=>
  object(Foo)#3 (3) {
    [%r"?d1"?:("Foo":)?private"?%r]=>
    object(Bar)#4 (3) {
      ["d1"]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
      [%r"?d2"?:("Bar":)?private"?%r]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
      [%r"?d3"?:protected"?%r]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
    }
    [%r"?d2"?:protected"?%r]=>
    object(Bar)#4 (3) {
      ["d1"]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
      [%r"?d2"?:("Bar":)?private"?%r]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
      [%r"?d3"?:protected"?%r]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
    }
    ["d3"]=>
    object(Bar)#4 (3) {
      ["d1"]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
      [%r"?d2"?:("Bar":)?private"?%r]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
      [%r"?d3"?:protected"?%r]=>
      object(Foo)#3 (3) {
        [%r"?d1"?:("Foo":)?private"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        [%r"?d2"?:protected"?%r]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          [%r"?d2"?:("Bar":)?private"?%r]=>
          *RECURSION*
          [%r"?d3"?:protected"?%r]=>
          *RECURSION*
        }
      }
    }
  }
  ["test"]=>
  string(6) "foobar"
  ["new"]=>
  object(Bar)#5 (3) {
    ["d1"]=>
    object(Foo)#6 (3) {
      [%r"?d1"?:("Foo":)?private"?%r]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      [%r"?d2"?:protected"?%r]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d3"]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
    }
    [%r"?d2"?:("Bar":)?private"?%r]=>
    object(Foo)#6 (3) {
      [%r"?d1"?:("Foo":)?private"?%r]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      [%r"?d2"?:protected"?%r]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d3"]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
    }
    [%r"?d3"?:protected"?%r]=>
    object(Foo)#6 (3) {
      [%r"?d1"?:("Foo":)?private"?%r]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      [%r"?d2"?:protected"?%r]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d3"]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d2"?:("Bar":)?private"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        [%r"?d3"?:protected"?%r]=>
        object(Foo)#6 (3) {
          [%r"?d1"?:("Foo":)?private"?%r]=>
          *RECURSION*
          [%r"?d2"?:protected"?%r]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
    }
  }
}
