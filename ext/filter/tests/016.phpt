--TEST--
filter_data() and FL_EMAIL
--GET--

--FILE--
<?php
$values = Array(
'a@b.c',	
'abuse@example.com',	
'test!.!@#$%^&*@example.com',	
'test@@#$%^&*())).com',	
'test@.com',	
'test@com',	
'@',	
'[]()/@example.com',	
'QWERTYUIOPASDFGHJKLZXCVBNM@QWERTYUIOPASDFGHJKLZXCVBNM.NET',	
);
foreach ($values as $value) {
	var_dump(filter_data($value, FL_EMAIL));
}

echo "Done\n";
?>
--EXPECT--	
string(5) "a@b.c"
string(17) "abuse@example.com"
NULL
NULL
NULL
NULL
NULL
NULL
string(57) "QWERTYUIOPASDFGHJKLZXCVBNM@QWERTYUIOPASDFGHJKLZXCVBNM.NET"
Done
