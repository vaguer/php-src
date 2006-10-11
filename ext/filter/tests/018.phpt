--TEST--
filter_var() and FILTER_VALIDATE_IP
--FILE--
<?php
var_dump(filter_var("192.168.0.1", FILTER_VALIDATE_IP));
var_dump(filter_var("192.168.0.1.1", FILTER_VALIDATE_IP));
var_dump(filter_var("::1", FILTER_VALIDATE_IP));
var_dump(filter_var("fe00::0", FILTER_VALIDATE_IP));
var_dump(filter_var("::123456", FILTER_VALIDATE_IP));
var_dump(filter_var("::1::b", FILTER_VALIDATE_IP));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP));
var_dump(filter_var("192.168.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE));
var_dump(filter_var("192.0.34.166", FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("192.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("192.0.34.166", FILTER_VALIDATE_IP));
var_dump(filter_var("256.1237.123.1", FILTER_VALIDATE_IP));
var_dump(filter_var("255.255.255.255", FILTER_VALIDATE_IP));
var_dump(filter_var("255.255.255.255", FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("", FILTER_VALIDATE_IP));
var_dump(filter_var(-1, FILTER_VALIDATE_IP));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV4));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("127.0.0.1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV4));
echo "Done\n";
?>
--EXPECT--	
string(11) "192.168.0.1"
NULL
string(3) "::1"
string(7) "fe00::0"
NULL
NULL
string(9) "127.0.0.1"
NULL
string(12) "192.0.34.166"
string(9) "127.0.0.1"
string(9) "192.0.0.1"
string(12) "192.0.34.166"
NULL
string(15) "255.255.255.255"
NULL
NULL
NULL
NULL
NULL
string(3) "::1"
string(9) "127.0.0.1"
Done
