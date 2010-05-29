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
write: 96a36f6c6484a24300a3466f6fa700466f6f00643184a24300a3426172a2643181a2720001a70042617200643281a2720001a5002a00643381a2720001a5002a00643281a2720002a2643381a2720002a474657374a6666f6f626172a36e657784a24300a3426172a2643184a24300a3466f6fa700466f6f00643181a2720009a5002a00643281a2720009a2643381a2720009a70042617200643281a272000aa5002a00643381a272000a
array(3) {
  ["old"]=>
  object(Foo)#3 (3) {
    ["d1":"Foo":private]=>
    object(Bar)#4 (3) {
      ["d1"]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
      ["d2":"Bar":private]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
      ["d3":protected]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
    }
    ["d2":protected]=>
    object(Bar)#4 (3) {
      ["d1"]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
      ["d2":"Bar":private]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
      ["d3":protected]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
    }
    ["d3"]=>
    object(Bar)#4 (3) {
      ["d1"]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
      ["d2":"Bar":private]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
      }
      ["d3":protected]=>
      object(Foo)#3 (3) {
        ["d1":"Foo":private]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d2":protected]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
          *RECURSION*
        }
        ["d3"]=>
        object(Bar)#4 (3) {
          ["d1"]=>
          *RECURSION*
          ["d2":"Bar":private]=>
          *RECURSION*
          ["d3":protected]=>
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
      ["d1":"Foo":private]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d2":protected]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d3"]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
    }
    ["d2":"Bar":private]=>
    object(Foo)#6 (3) {
      ["d1":"Foo":private]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d2":protected]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d3"]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
    }
    ["d3":protected]=>
    object(Foo)#6 (3) {
      ["d1":"Foo":private]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d2":protected]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
      ["d3"]=>
      object(Bar)#5 (3) {
        ["d1"]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d2":"Bar":private]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
        ["d3":protected]=>
        object(Foo)#6 (3) {
          ["d1":"Foo":private]=>
          *RECURSION*
          ["d2":protected]=>
          *RECURSION*
          ["d3"]=>
          *RECURSION*
        }
      }
    }
  }
}
