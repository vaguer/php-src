--TEST--
Closure 013: __invoke() on temporary result
--SKIPIF--
<?php 
	if (!class_exists('Closure')) die('skip Closure support is needed');
?>
--FILE--
<?php
class Foo {
	function __invoke() {
		echo "Hello World!\n";
	}
}

function foo() {
	return function() {
		echo "Hello World!\n";
	};
}
$test = new Foo;
$test->__invoke();
$test = foo();
$test->__invoke();
$test = foo()->__invoke();
?>
--EXPECT--
Hello World!
Hello World!
Hello World!
