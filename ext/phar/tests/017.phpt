--TEST--
Phar: opendir test - no dir specified at all
--SKIPIF--
<?php if (!extension_loaded("phar")) print "skip"; ?>
--INI--
phar.require_hash=0
--FILE--
<?php
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";
// file length is too short

$files = array();
$files['a'] = 'abc';
$manifest = '';
foreach($files as $name => $cont) {
	$len = strlen($cont);
	$manifest .= pack('V', strlen($name)) . $name . pack('VVVVVV', $len, time(), $len, crc32($cont), 0x00000001, 0);
}
$alias = 'hio';
$manifest = pack('VnVV', count($files), 0x0900, 0x00000001, strlen($alias)) . $alias . $manifest;
$file .= pack('V', strlen($manifest)) . $manifest;
foreach($files as $cont)
{
	$file .= $cont;
}

file_put_contents(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php', $file);
include dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$dir = opendir('phar://hio');
?>
--CLEAN--
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar.php'); ?>
--EXPECTF--
Warning: opendir(phar://hio): failed to open dir: phar error: no directory in "phar://hio", must have at least phar://hio/ for root directory in %s on line %d