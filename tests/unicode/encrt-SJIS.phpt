--TEST--
declare script encoding (SJIS)
--INI--
unicode.semantics=on
unicode.output_encoding=CP866
--FILE--
<?php
declare(encoding="SJIS");

function u() {
  echo "u - ok\n";
}

u();
?>
--EXPECT--
β₯αβ - ok
