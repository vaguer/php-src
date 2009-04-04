--TEST--
method overloading with different method signature
--INI--
error_reporting=8191
--FILE--
<?php

class test {
	function foo(Test $arg) {}
}

class test2 extends test {
	function foo(Test $arg) {} 
}

class test3 extends test {
	function foo($arg) {} 
}

echo "Done\n";
?>
--EXPECTF--
Strict Standards: Declaration of test3::foo() should be compatible with that of test::foo() in %s on line %d
Done
