<?php

$notes = '
 * add Phar::getAlias() [Marcus]
 * Made -a optional in pack subcommand of phar.phar [Marcus]
 * Fix issue with apache module and extracted archives [Marcus]
 * Send all error messages to stderr in phar.phar [Marcus]
 * Added new subcommands add and delete to phar.phar [Marcus]
 * Made Phar::loadPhar() and Phar::mapPhar() ignore extracted archives [Marcus]
 * Fix issue with compressed entries and uncompressing entries [Marcus]
 * Verify stubs before writing [Marcus]
 * Always use longest stub end to avoid issues with length field [Marcus]
';

if (!class_exists("Phar") && !extension_loaded("Phar")) {
	die("Extension phar not present");
}

require_once 'PEAR/PackageFileManager2.php';

PEAR::setErrorHandling(PEAR_ERROR_DIE);

$options = array(
    'filelistgenerator' => 'CVS',
    'changelogoldtonew' => false,
    'simpleoutput'      => true,
    'baseinstalldir'    => '/',
    'packagedirectory'  => dirname(__FILE__),
    'packagefile'       => 'package.xml',
    'clearcontents'     => true,
    'ignore'            => array('package*.php', 'package*.xml'),
    'dir_roles'         => array(
         'docs'         => 'doc',
         'examples'     => 'doc',
         'tests'        => 'test',
         'phar'         => 'src',
    ),
    'exceptions'        => array(
         'CREDITS'            => 'doc',
         'EXPERIMENTAL'       => 'doc',
         'LICENSE'            => 'doc',
         'Makefile.frag'      => 'src',
         'phar_path_check.re' => 'src',
         'TODO'               => 'doc',
    ),
);

$package = PEAR_PackageFileManager2::importOptions(dirname(__FILE__) . '/package.xml', $options);

$package->clearDeps();
$package->setPhpDep('5.2.0');
$package->setPearInstallerDep('1.4.3');
$package->addPackageDepWithChannel('optional', 'bz2', 'pecl.php.net', false, false, false, false, 'bz2');
// all this false business sets the <providesextension> tag that allows us to have hash built
// in statically
$package->addPackageDepWithChannel('optional', 'hash', 'pecl.php.net', false, false, false, false, 'hash');
$package->addExtensionDep('optional', 'spl');
$package->addExtensionDep('optional', 'zlib');
$package->setPackageType('extsrc');
$package->addRelease();
$package->setReleaseVersion(phpversion('phar'));
$package->setAPIVersion(Phar::apiVersion());
$package->setReleaseStability('stable');
$package->setAPIStability('stable');
$package->setNotes("\n$notes\n");
//$package->addGlobalReplacement('package-info', '@package_version@', 'version');
$package->generateContents();

if (isset($_GET['make']) || (isset($_SERVER['argv']) && @$_SERVER['argv'][1] == 'make')) {
    $package->writePackageFile();
} else {
    $package->debugPackageFile();
}

?>
