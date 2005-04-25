--TEST--
parse_url() function
--FILE--
<?php
$sample_urls = array (
'',
'64.246.30.37',
'http://64.246.30.37',
'http://64.246.30.37/',
'64.246.30.37/',
'64.246.30.37:80/',
'php.net',
'php.net/',
'http://php.net',
'http://php.net/',
'www.php.net',
'www.php.net/',
'http://www.php.net',
'http://www.php.net/',
'www.php.net:80',
'http://www.php.net:80',
'http://www.php.net:80/',
'http://www.php.net/index.php',
'www.php.net/?',
'www.php.net:80/?',
'http://www.php.net/?',
'http://www.php.net:80/?',
'http://www.php.net:80/index.php',
'http://www.php.net:80/foo/bar/index.php',
'http://www.php.net:80/this/is/a/very/deep/directory/structure/and/file.php',
'http://www.php.net:80/this/is/a/very/deep/directory/structure/and/file.php?lots=1&of=2&parameters=3&too=4&here=5',
'http://www.php.net:80/this/is/a/very/deep/directory/structure/and/',
'http://www.php.net:80/this/is/a/very/deep/directory/structure/and/file.php',
'http://www.php.net:80/this/../a/../deep/directory',
'http://www.php.net:80/this/../a/../deep/directory/',
'http://www.php.net:80/this/is/a/very/deep/directory/../file.php',
'http://www.php.net:80/index.php',
'http://www.php.net:80/index.php?',
'http://www.php.net:80/#foo',
'http://www.php.net:80/?#',
'http://www.php.net:80/?test=1',
'http://www.php.net/?test=1&',
'http://www.php.net:80/?&',
'http://www.php.net:80/index.php?test=1&',
'http://www.php.net/index.php?&',
'http://www.php.net:80/index.php?foo&',
'http://www.php.net/index.php?&foo',
'http://www.php.net:80/index.php?test=1&test2=char&test3=mixesCI',
'www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'http://secret@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'http://secret:@www.php.net/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'http://:hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'http://secret:hideout@www.php.net/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'http://secret@hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'http://secret:hid:out@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123',
'nntp://news.php.net',
'ftp://ftp.gnu.org/gnu/glic/glibc.tar.gz',
'zlib:http://foo@bar',
'zlib:filename.txt',
'zlib:/path/to/my/file/file.txt',
'foo://foo@bar',
'mailto:me@mydomain.com',
'/foo.php?a=b&c=d',
'foo.php?a=b&c=d',
'http://user:passwd@www.example.com:8080?bar=1&boom=0',
'file:///path/to/file',
'file://path/to/file',
'file:/path/to/file',
'http://1.2.3.4:/abc.asp?a=1&b=2',
'http://foo.com#bar',
'scheme:'
);

    foreach ($sample_urls as $url) {
        var_dump(@parse_url($url));
    }
?>
--EXPECT--
array(1) {
  ["path"]=>
  string(0) ""
}
array(1) {
  ["path"]=>
  string(12) "64.246.30.37"
}
array(2) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(12) "64.246.30.37"
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(12) "64.246.30.37"
  ["path"]=>
  string(1) "/"
}
array(1) {
  ["path"]=>
  string(13) "64.246.30.37/"
}
array(3) {
  ["host"]=>
  string(12) "64.246.30.37"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
}
array(1) {
  ["path"]=>
  string(7) "php.net"
}
array(1) {
  ["path"]=>
  string(8) "php.net/"
}
array(2) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(7) "php.net"
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(7) "php.net"
  ["path"]=>
  string(1) "/"
}
array(1) {
  ["path"]=>
  string(11) "www.php.net"
}
array(1) {
  ["path"]=>
  string(12) "www.php.net/"
}
array(2) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["path"]=>
  string(1) "/"
}
array(2) {
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["path"]=>
  string(10) "/index.php"
}
array(1) {
  ["path"]=>
  string(12) "www.php.net/"
}
array(3) {
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["path"]=>
  string(1) "/"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(18) "/foo/bar/index.php"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(53) "/this/is/a/very/deep/directory/structure/and/file.php"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(53) "/this/is/a/very/deep/directory/structure/and/file.php"
  ["query"]=>
  string(37) "lots=1&of=2&parameters=3&too=4&here=5"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(45) "/this/is/a/very/deep/directory/structure/and/"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(53) "/this/is/a/very/deep/directory/structure/and/file.php"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(28) "/this/../a/../deep/directory"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(29) "/this/../a/../deep/directory/"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(42) "/this/is/a/very/deep/directory/../file.php"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
  ["fragment"]=>
  string(3) "foo"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
  ["query"]=>
  string(6) "test=1"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["path"]=>
  string(1) "/"
  ["query"]=>
  string(7) "test=1&"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(1) "/"
  ["query"]=>
  string(1) "&"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(7) "test=1&"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(1) "&"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(4) "foo&"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(4) "&foo"
}
array(5) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
}
array(5) {
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(7) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["user"]=>
  string(6) "secret"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(6) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["user"]=>
  string(6) "secret"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(7) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["pass"]=>
  string(7) "hideout"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(7) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["user"]=>
  string(6) "secret"
  ["pass"]=>
  string(7) "hideout"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(7) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(19) "hideout@www.php.net"
  ["port"]=>
  int(80)
  ["user"]=>
  string(6) "secret"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(8) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "www.php.net"
  ["port"]=>
  int(80)
  ["user"]=>
  string(6) "secret"
  ["pass"]=>
  string(7) "hid:out"
  ["path"]=>
  string(10) "/index.php"
  ["query"]=>
  string(31) "test=1&test2=char&test3=mixesCI"
  ["fragment"]=>
  string(16) "some_page_ref123"
}
array(2) {
  ["scheme"]=>
  string(4) "nntp"
  ["host"]=>
  string(12) "news.php.net"
}
array(3) {
  ["scheme"]=>
  string(3) "ftp"
  ["host"]=>
  string(11) "ftp.gnu.org"
  ["path"]=>
  string(22) "/gnu/glic/glibc.tar.gz"
}
array(2) {
  ["scheme"]=>
  string(4) "zlib"
  ["path"]=>
  string(14) "http://foo@bar"
}
array(2) {
  ["scheme"]=>
  string(4) "zlib"
  ["path"]=>
  string(12) "filename.txt"
}
array(2) {
  ["scheme"]=>
  string(4) "zlib"
  ["path"]=>
  string(25) "/path/to/my/file/file.txt"
}
array(3) {
  ["scheme"]=>
  string(3) "foo"
  ["host"]=>
  string(3) "bar"
  ["user"]=>
  string(3) "foo"
}
array(2) {
  ["scheme"]=>
  string(6) "mailto"
  ["path"]=>
  string(15) "me@mydomain.com"
}
array(2) {
  ["path"]=>
  string(8) "/foo.php"
  ["query"]=>
  string(7) "a=b&c=d"
}
array(2) {
  ["path"]=>
  string(7) "foo.php"
  ["query"]=>
  string(7) "a=b&c=d"
}
array(6) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(15) "www.example.com"
  ["port"]=>
  int(8080)
  ["user"]=>
  string(4) "user"
  ["pass"]=>
  string(6) "passwd"
  ["query"]=>
  string(12) "bar=1&boom=0"
}
array(2) {
  ["scheme"]=>
  string(4) "file"
  ["path"]=>
  string(13) "/path/to/file"
}
array(3) {
  ["scheme"]=>
  string(4) "file"
  ["host"]=>
  string(4) "path"
  ["path"]=>
  string(8) "/to/file"
}
array(2) {
  ["scheme"]=>
  string(4) "file"
  ["path"]=>
  string(13) "/path/to/file"
}
array(4) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(7) "1.2.3.4"
  ["path"]=>
  string(8) "/abc.asp"
  ["query"]=>
  string(7) "a=1&b=2"
}
array(3) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(7) "foo.com"
  ["fragment"]=>
  string(3) "bar"
}
array(1) {
  ["scheme"]=>
  string(6) "scheme"
}
