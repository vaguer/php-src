<?php

function hallo() {
}

function simpleucall($n) {
  for ($i = 0; $i < $n; $i++) 
    hallo();
}

function simpleudcall($n) {
  for ($i = 0; $i < $n; $i++) 
    hallo2();
}

function hallo2() {
}

function simpleicall($n) {
  for ($i = 0; $i < $n; $i++) 
    func_num_args();
}

class Foo {
	static $a = 0;
	public $b = 0;
	const TEST = 0;

	static function read_static($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = self::$a;
		}
	}

	static function write_static($n) {
		for ($i = 0; $i < $n; ++$i) {
			self::$a = 0;
		}
	}

	static function isset_static($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = isset(self::$a);
		}
	}

	static function empty_static($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = empty(self::$a);
		}
	}

	static function f() {
	}

	static function call_static($n) {
		for ($i = 0; $i < $n; ++$i) {
			self::f();
		}
	}

	function read_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = $this->b;
		}
	}

	function write_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$this->b = 0;
		}
	}

	function assign_add_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$this->b += 2;
		}
	}

	function pre_inc_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			++$this->b;
		}
	}

	function pre_dec_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			--$this->b;
		}
	}

	function post_inc_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$this->b++;
		}
	}

	function post_dec_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$this->b--;
		}
	}

	function isset_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = isset($this->b);
		}
	}

	function empty_prop($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = empty($this->b);
		}
	}

	function g() {
	}

	function call($n) {
		for ($i = 0; $i < $n; ++$i) {
			$this->g();
		}
	}

	function read_const($n) {
		for ($i = 0; $i < $n; ++$i) {
			$x = $this::TEST;
		}
	}

}

function read_static($n) {
	for ($i = 0; $i < $n; ++$i) {
		$x = Foo::$a;
	}
}

function write_static($n) {
	for ($i = 0; $i < $n; ++$i) {
		Foo::$a = 0;
	}
}

function isset_static($n) {
	for ($i = 0; $i < $n; ++$i) {
		$x = isset(Foo::$a);
	}
}

function empty_static($n) {
	for ($i = 0; $i < $n; ++$i) {
		$x = empty(Foo::$a);
	}
}

function call_static($n) {
	for ($i = 0; $i < $n; ++$i) {
		Foo::f();
	}
}

function create_object($n) {
	for ($i = 0; $i < $n; ++$i) {
		$x = new Foo();
	}
}

define('TEST', null);

function read_const($n) {
	for ($i = 0; $i < $n; ++$i) {
		$x = TEST;
	}
}

/*****/

function empty_loop($n) {
	for ($i = 0; $i < $n; ++$i) {
	}
}

function getmicrotime()
{
  $t = gettimeofday();
  return ($t['sec'] + $t['usec'] / 1000000);
}

function start_test()
{
  ob_start();
  return getmicrotime();
}

function end_test($start, $name, $overhead = null)
{
  global $total;
  global $last_time;
  $end = getmicrotime();
  ob_end_clean();
  $last_time = $end-$start;
  $total += $last_time;
  $num = number_format($last_time,3);
  $pad = str_repeat(" ", 24-strlen($name)-strlen($num));
  if (is_null($overhead)) {
    echo $name.$pad.$num."\n";
  } else {
    $num2 = number_format($last_time - $overhead,3);
    echo $name.$pad.$num."    ".$num2."\n";
  }
  ob_start();
  return getmicrotime();
}

function total()
{
  global $total;
  $pad = str_repeat("-", 24);
  echo $pad."\n";
  $num = number_format($total,3);
  $pad = str_repeat(" ", 24-strlen("Total")-strlen($num));
  echo "Total".$pad.$num."\n";
}

const N = 5000000;

$t0 = $t = start_test();
empty_loop(N);
$t = end_test($t, 'empty_loop');
$overhead = $last_time;
simpleucall(N);
$t = end_test($t, 'func()', $overhead);
simpleudcall(N);
$t = end_test($t, 'undef_func()', $overhead);
simpleicall(N);
$t = end_test($t, 'int_func()', $overhead);
Foo::read_static(N);
$t = end_test($t, '$x = self::$x', $overhead);
Foo::write_static(N);
$t = end_test($t, 'self::$x = 0', $overhead);
Foo::isset_static(N);
$t = end_test($t, 'isset(self::$x)', $overhead);
Foo::empty_static(N);
$t = end_test($t, 'empty(self::$x)', $overhead);
read_static(N);
$t = end_test($t, '$x = Foo::$x', $overhead);
write_static(N);
$t = end_test($t, 'Foo::$x = 0', $overhead);
isset_static(N);
$t = end_test($t, 'isset(Foo::$x)', $overhead);
empty_static(N);
$t = end_test($t, 'empty(Foo::$x)', $overhead);
Foo::call_static(N);
$t = end_test($t, 'self::f()', $overhead);
call_static(N);
$t = end_test($t, 'Foo::f()', $overhead);
$x = new Foo();
$x->read_prop(N);
$t = end_test($t, '$x = $this->x', $overhead);
$x->write_prop(N);
$t = end_test($t, '$this->x = 0', $overhead);
$x->assign_add_prop(N);
$t = end_test($t, '$this->x += 2', $overhead);
$x->pre_inc_prop(N);
$t = end_test($t, '++$this->x', $overhead);
$x->pre_dec_prop(N);
$t = end_test($t, '--$this->x', $overhead);
$x->post_inc_prop(N);
$t = end_test($t, '$this->x++', $overhead);
$x->post_dec_prop(N);
$t = end_test($t, '$this->x--', $overhead);
$x->isset_prop(N);
$t = end_test($t, 'isset($this->x)', $overhead);
$x->empty_prop(N);
$t = end_test($t, 'empty($this->x)', $overhead);
$x->call(N);
$t = end_test($t, '$this->f()', $overhead);
$x->read_const(N);
$t = end_test($t, '$x = Foo::TEST', $overhead);
create_object(N);
$t = end_test($t, 'new Foo()', $overhead);
read_const(N);
$t = end_test($t, '$x = TEST', $overhead);
total($t0, "Total");
