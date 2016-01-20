In this test, it measured the elapsed time of serializing and deserializing.

If the elapsed time will be performed 10,000 times.

# Version #
  * PHP 5.3.3 (DEBUG)
  * Extension 0.3.3

# Functions #
|               | **default**       | **json**          | **msgpack**               |
|:--------------|:------------------|:------------------|:--------------------------|
| **serialize**   | _serialize()_     | _json\_encode()_  | _msgpack\_serialize()_    |
| **unserialize** | _unserialize()_   | _json\_decode()_  | _msgpack\_unserialize()_  |

# Benchmark #
```
require_once 'Benchmark/Timer.php';

$loop = 10000;

$pack = null;
$unpack = null;

$t = new Benchmark_Timer;
$t->start();
for ($i = 0; $i < $loop; $i++)
{
    $pack = msgpack_serialize($value);
    //$pack = serialize($value);
    //$pack = json_encode($value);
}
$t->setMarker('serialize');

for ($i = 0; $i < $loop; $i++)
{
    $unpack = msgpack_unserialize($pack);
    //$unpack = unserialize($pack);
    //$unpack = json_decode($pack);
}

$t->stop();
$t->display();
```

# Performances #
## Integer ##
### Value ###
```
$values = rand();
```

### Speed (sec) ###
|               | **default**       | **json**          | **msgpack**       |
|:--------------|:------------------|:------------------|:------------------|
| **serialize**   | 0.0373 _(100%)_   | 0.0288 _(77%)_    | 0.0314 _( 84%)_   |
| **unserialize** | 0.0323 _(100%)_   | 0.0640 _(198%)_   | 0.0340 _(105%)_   |

### Size (byte) ###
| **default**   | **json**    | **msgpack** |
|:--------------|:------------|:------------|
| 12 _(100%)_   | 9 _(75%)_   | 5 _(41%)_   |


## Floating point number ##
### Value ###
```
$value = log(rand());
```

### Speed (sec) ###
|               | **default**       | **json**         | **msgpack**      |
|:--------------|:------------------|:-----------------|:-----------------|
| **serialize**   | 0.1975 _(100%)_   | 0.0404 _(20%)_   | 0.0293 _(14%)_   |
| **unserialize** | 0.1193 _(100%)_   | 0.0560 _(46%)_   | 0.0312 _(26%)_   |

### Size (byte) ###
| **default**   | **json**     | **msgpack** |
|:--------------|:-------------|:------------|
| 54 _(100%)_   | 15 _(27%)_   | 9 _(16%)_   |


## String ##
### Value ###
```
$value = md5(rand());
```

### Speed (sec) ###
|               | **default**       | **json**          | **msgpack**       |
|:--------------|:------------------|:------------------|:------------------|
| **serialize**   | 0.0282 _(100%)_   | 0.0527 _(187%)_   | 0.0263 _(93%)_    |
| **unserialize** | 0.0337 _(100%)_   | 0.0708 _(209%)_   | 0.0336 _(99%)_    |

### Size (byte) ###
| **default**   | **json**     | **msgpack**  |
|:--------------|:-------------|:-------------|
| 40 _(100%)_   | 34 _(85%)_   | 35 _(87%)_   |


## Array ##
### Value ###
```
$value = array(md5(rand()),
               md5(rand()),
               md5(rand()),
               md5(rand()),
               md5(rand()));
```

### Speed (sec) ###
|               | **default**       | **json**          | **msgpack**      |
|:--------------|:------------------|:------------------|:-----------------|
| **serialize**   | 0.1130 _(100%)_   | 0.2403 _(212%)_   | 0.0635 _(56%)_   |
| **unserialize** | 0.1372 _(100%)_   | 0.3507 _(255%)_   | 0.1682 _(122%)_  |

### Size (byte) ###
| **default**    | **json**      | **msgpack**   |
|:---------------|:--------------|:--------------|
| 226 _(100%)_   | 176 _(77%)_   | 181 _(80%)_   |


## Array (Hash) ##
### Value ###
```
$value = array(md5(rand()) => md5(rand()),
               md5(rand()) => md5(rand()),
               md5(rand()) => md5(rand()),
               md5(rand()) => md5(rand()),
               md5(rand()) => md5(rand()));
```

### Speed (sec) ###
|               | **default**       | **json**          | **msgpack**      |
|:--------------|:------------------|:------------------|:-----------------|
| **serialize**   | 0.1277 _(100%)_   | 0.4457 _(348%)_   | 0.0706 _(55%)_   |
| **unserialize** | 0.2026 _(100%)_   | 0.6131 _(302%)_   | 0.2185 _(107%)_  |

### Size (byte) ###
| **default**    | **json**      | **msgpack**  |
|:---------------|:--------------|:-------------|
| 406 _(100%)_   | 351 _(86%)_   | 351 _(86%)_  |


## Object ##
### Value ###
```
$value = new stdClass;
$value->param1 = rand();
$value->param2 = md5(uniqid());
$value->param3 = array(md5(uniqid()));
$value->param4 = array(md5(uniqid()) => md5(uniqid()));
$value->param5 = null;
```

### Speed (sec) ###
|               | **default**       | **json** | **msgpack**      |
|:--------------|:------------------|:---------|:-----------------|
| **serialize**   | 0.1788 _(100%)_   | -        | 0.1114 _(62%)_   |
| **unserialize** | 0.2862 _(100%)_   | -        | 0.2980 _(104%)_  |

### Size (byte) ###
| **default**    | **json** | **msgpack**   |
|:---------------|:---------|:--------------|
| 275 _(100%)_   | -        | 195 _(70%)_   |