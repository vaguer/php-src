--TEST--
Bug #22207 (missing 0 when using the e notation in *printf functions)
--FILE--
<?php
	printf("%10.5e\n", 1.1); 
	var_dump(sprintf("%10.5e\n", 1.1));
?>
--EXPECT--
1.1000e+0
string(17) "       1.1000e+0
"
