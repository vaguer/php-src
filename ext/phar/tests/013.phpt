--TEST--
PHP_Archive::mapPhar filesize mismatch
--SKIPIF--
<?php if (!extension_loaded("phar")) print "skip";?>
--FILE--
<?php
function cleanup() { unlink(dirname(__FILE__) . '/008_phar.php'); }
register_shutdown_function('cleanup');
$file = "<?php
PHP_Archive::mapPhar(5, 'hio', false);
__HALT_COMPILER(); ?>";
// filesize should be 1, and is 2
$manifest = pack('V', 1) . 'a' . pack('VVVV', 1, time(), 0, 9);
$file .= pack('VV', strlen($manifest) + 4, 1) . $manifest . pack('VV', crc32('a'), 2) . 'a';
file_put_contents(dirname(__FILE__) . '/008_phar.php', $file);
include dirname(__FILE__) . '/008_phar.php';
echo file_get_contents('phar://hio/a');
?>
--EXPECTF--
Warning: file_get_contents(phar://hio/a): failed to open stream: phar error: internal corruption of phar "%s" (filesize mismatch on file "a") in %s on line 12