/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2008 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Thies C. Arntzen <thies@thieso.net>                          |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/* {{{ includes/startup/misc */

#include "php.h"
#include "fopen_wrappers.h"
#include "file.h"
#include "php_dir.h"
#include "php_string.h"
#include "php_scandir.h"

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>

#ifdef PHP_WIN32
#include "win32/readdir.h"
#endif


#ifdef HAVE_GLOB
#ifndef PHP_WIN32
#include <glob.h>
#else
#include "win32/glob.h"
#endif
#endif

typedef struct {
	int default_dir;
} php_dir_globals;

#ifdef ZTS
#define DIRG(v) TSRMG(dir_globals_id, php_dir_globals *, v)
int dir_globals_id;
#else
#define DIRG(v) (dir_globals.v)
php_dir_globals dir_globals;
#endif

#if 0
typedef struct {
	int id;
	DIR *dir;
} php_dir;

static int le_dirp;
#endif

static zend_class_entry *dir_class_entry_ptr;

#define FETCH_DIRP() \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &id) == FAILURE) { \
		return; \
	} \
	if (ZEND_NUM_ARGS() == 0) { \
		myself = getThis(); \
		if (myself) { \
			if (zend_hash_find(Z_OBJPROP_P(myself), "handle", sizeof("handle"), (void **)&tmp) == FAILURE) { \
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find my handle property"); \
				RETURN_FALSE; \
			} \
			ZEND_FETCH_RESOURCE(dirp, php_stream *, tmp, -1, "Directory", php_file_le_stream()); \
		} else { \
			ZEND_FETCH_RESOURCE(dirp, php_stream *, 0, DIRG(default_dir), "Directory", php_file_le_stream()); \
		} \
	} else { \
		dirp = (php_stream *) zend_fetch_resource(&id TSRMLS_CC, -1, "Directory", NULL, 1, php_file_le_stream()); \
		if (!dirp) \
			RETURN_FALSE; \
	} 

static const zend_function_entry php_dir_class_functions[] = {
	PHP_FALIAS(close,	closedir,	NULL)
	PHP_FALIAS(rewind,	rewinddir,	NULL)
	PHP_NAMED_FE(read,  php_if_readdir, NULL)
	{NULL, NULL, NULL}
};


static void php_set_default_dir(int id TSRMLS_DC)
{
	if (DIRG(default_dir)!=-1) {
		zend_list_delete(DIRG(default_dir));
	}

	if (id != -1) {
		zend_list_addref(id);
	}
	
	DIRG(default_dir) = id;
}

PHP_RINIT_FUNCTION(dir)
{
	DIRG(default_dir) = -1;
	return SUCCESS;
}

PHP_MINIT_FUNCTION(dir)
{
	static char dirsep_str[2], pathsep_str[2];
	zend_class_entry dir_class_entry;

	INIT_CLASS_ENTRY(dir_class_entry, "Directory", php_dir_class_functions);
	dir_class_entry_ptr = zend_register_internal_class(&dir_class_entry TSRMLS_CC);

#ifdef ZTS
	ts_allocate_id(&dir_globals_id, sizeof(php_dir_globals), NULL, NULL);
#endif

	dirsep_str[0] = DEFAULT_SLASH;
	dirsep_str[1] = '\0';
	REGISTER_STRING_CONSTANT("DIRECTORY_SEPARATOR", dirsep_str, CONST_CS|CONST_PERSISTENT);

	pathsep_str[0] = ZEND_PATHS_SEPARATOR;
	pathsep_str[1] = '\0';
	REGISTER_STRING_CONSTANT("PATH_SEPARATOR", pathsep_str, CONST_CS|CONST_PERSISTENT);

#ifdef HAVE_GLOB

#ifdef GLOB_BRACE
	REGISTER_LONG_CONSTANT("GLOB_BRACE", GLOB_BRACE, CONST_CS | CONST_PERSISTENT);
#else
# define GLOB_BRACE 0
#endif

#ifdef GLOB_MARK
	REGISTER_LONG_CONSTANT("GLOB_MARK", GLOB_MARK, CONST_CS | CONST_PERSISTENT);
#else
# define GLOB_MARK 0
#endif

#ifdef GLOB_NOSORT
	REGISTER_LONG_CONSTANT("GLOB_NOSORT", GLOB_NOSORT, CONST_CS | CONST_PERSISTENT);
#else 
# define GLOB_NOSORT 0
#endif

#ifdef GLOB_NOCHECK
	REGISTER_LONG_CONSTANT("GLOB_NOCHECK", GLOB_NOCHECK, CONST_CS | CONST_PERSISTENT);
#else 
# define GLOB_NOCHECK 0
#endif

#ifdef GLOB_NOESCAPE
	REGISTER_LONG_CONSTANT("GLOB_NOESCAPE", GLOB_NOESCAPE, CONST_CS | CONST_PERSISTENT);
#else 
# define GLOB_NOESCAPE 0
#endif

#ifdef GLOB_ERR
	REGISTER_LONG_CONSTANT("GLOB_ERR", GLOB_ERR, CONST_CS | CONST_PERSISTENT);
#else 
# define GLOB_ERR 0
#endif

#ifndef GLOB_ONLYDIR
# define GLOB_ONLYDIR (1<<30)
# define GLOB_EMULATE_ONLYDIR
# define GLOB_FLAGMASK (~GLOB_ONLYDIR)
#else
# define GLOB_FLAGMASK (~0)
#endif

/* This is used for checking validity of passed flags (passing invalid flags causes segfault in glob()!! */
#define GLOB_AVAILABLE_FLAGS (0 | GLOB_BRACE | GLOB_MARK | GLOB_NOSORT | GLOB_NOCHECK | GLOB_NOESCAPE | GLOB_ERR | GLOB_ONLYDIR)

	REGISTER_LONG_CONSTANT("GLOB_ONLYDIR", GLOB_ONLYDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GLOB_AVAILABLE_FLAGS", GLOB_AVAILABLE_FLAGS, CONST_CS | CONST_PERSISTENT);

#endif /* HAVE_GLOB */

	return SUCCESS;
}
/* }}} */

/* {{{ internal functions */
static void _php_do_opendir(INTERNAL_FUNCTION_PARAMETERS, int createobject)
{
	zval **ppdir;
	UChar *udir = NULL;
	char *dir;
	int dir_len, udir_len;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;
	php_stream *dirp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|r", &ppdir, &zcontext) == FAILURE) {
		return;
	}

	RETVAL_FALSE;

	if (createobject && Z_TYPE_PP(ppdir) == IS_UNICODE) {
		/* Save for later */
		udir = eustrndup(Z_USTRVAL_PP(ppdir), Z_USTRLEN_PP(ppdir));
		udir_len = Z_USTRLEN_PP(ppdir);
	}

	context = php_stream_context_from_zval(zcontext, 0);
	if (FAILURE == php_stream_path_param_encode(ppdir, &dir, &dir_len, REPORT_ERRORS, context)) {
		goto opendir_cleanup;
	}

	dirp = php_stream_opendir(dir, REPORT_ERRORS, context);
	if (dirp == NULL) {
		goto opendir_cleanup;
	}

	dirp->flags |= PHP_STREAM_FLAG_NO_FCLOSE;
		
	php_set_default_dir(dirp->rsrc_id TSRMLS_CC);

	if (createobject) {
		object_init_ex(return_value, dir_class_entry_ptr);
		if (udir) {
			add_property_unicodel(return_value, "path", udir, udir_len, 0);

			/* Avoid auto-cleanup */
			udir = NULL;
		} else {
			add_property_stringl(return_value, "path", dir, dir_len, 1);
		}
		add_property_resource(return_value, "handle", dirp->rsrc_id);
		php_stream_auto_cleanup(dirp); /* so we don't get warnings under debug */
	} else {
		php_stream_to_zval(dirp, return_value);
	}

opendir_cleanup:
	if (udir) {
		efree(udir);
	}
}
/* }}} */

/* {{{ proto mixed opendir(string path[, resource context]) U
   Open a directory and return a dir_handle */
PHP_FUNCTION(opendir)
{
	_php_do_opendir(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto object dir(string directory[, resource context]) U
   Directory class with properties, handle and class and methods read, rewind and close */
PHP_FUNCTION(getdir)
{
	_php_do_opendir(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto void closedir([resource dir_handle]) U
   Close directory connection identified by the dir_handle */
PHP_FUNCTION(closedir)
{
	zval *id = NULL, **tmp, *myself;
	php_stream *dirp;
	int rsrc_id;

	FETCH_DIRP();

	if (!(dirp->flags & PHP_STREAM_FLAG_IS_DIR)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d is not a valid Directory resource", dirp->rsrc_id);
		RETURN_FALSE;
	}

	rsrc_id = dirp->rsrc_id;
	zend_list_delete(dirp->rsrc_id);

	if (rsrc_id == DIRG(default_dir)) {
		php_set_default_dir(-1 TSRMLS_CC);
	}
}
/* }}} */

#if defined(HAVE_CHROOT) && !defined(ZTS) && ENABLE_CHROOT_FUNC
/* {{{ proto bool chroot(string directory) U
   Change root directory */
PHP_FUNCTION(chroot)
{
	zval **ppstr;
	char *str;
	int ret, str_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z", &ppstr) == FAILURE ||
		php_stream_path_param_encode(ppstr, &str, &str_len, REPORT_ERRORS, FG(default_context)) == FAILURE) {
		return;
	}
	
	ret = chroot(str);
	if (ret != 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s (errno %d)", strerror(errno), errno);
		RETURN_FALSE;
	}

	realpath_cache_clean(TSRMLS_C);
	
	ret = chdir("/");
	
	if (ret != 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s (errno %d)", strerror(errno), errno);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ proto bool chdir(string directory) U
   Change the current directory */
PHP_FUNCTION(chdir)
{
	zval **ppstr;
	char *str;
	int ret, str_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z", &ppstr) == FAILURE ||
		php_stream_path_param_encode(ppstr, &str, &str_len, REPORT_ERRORS, FG(default_context)) == FAILURE) {
		return;
	}
	if (php_check_open_basedir(str TSRMLS_CC)) {
  		RETURN_FALSE;
  	}
	ret = VCWD_CHDIR(str);
	if (ret != 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s (errno %d)", strerror(errno), errno);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed getcwd(void) U
   Gets the current directory */
PHP_FUNCTION(getcwd)
{
	char path[MAXPATHLEN];
	char *ret=NULL;
	
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if HAVE_GETCWD
	ret = VCWD_GETCWD(path, MAXPATHLEN);
#elif HAVE_GETWD
	ret = VCWD_GETWD(path);
#endif

	if (ret) {
		RETURN_RT_STRING(path, ZSTR_DUPLICATE);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void rewinddir([resource dir_handle]) U
   Rewind dir_handle back to the start */
PHP_FUNCTION(rewinddir)
{
	zval *id = NULL, **tmp, *myself;
	php_stream *dirp;
	
	FETCH_DIRP();

	if (!(dirp->flags & PHP_STREAM_FLAG_IS_DIR)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d is not a valid Directory resource", dirp->rsrc_id);
		RETURN_FALSE;
	}

	php_stream_rewinddir(dirp);
}
/* }}} */

/* {{{ proto string readdir([resource dir_handle]) U
   Read directory entry from dir_handle */
PHP_NAMED_FUNCTION(php_if_readdir)
{
	zval *id = NULL, **tmp, *myself;
	php_stream *dirp;
	php_stream_dirent entry;

	FETCH_DIRP();

	if (!(dirp->flags & PHP_STREAM_FLAG_IS_DIR)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d is not a valid Directory resource", dirp->rsrc_id);
		RETURN_FALSE;
	}

	if (php_stream_readdir(dirp, &entry)) {
		RETURN_RT_STRINGL(entry.d_name, strlen(entry.d_name), ZSTR_DUPLICATE);
	}
	RETURN_FALSE;
}
/* }}} */

#ifdef HAVE_GLOB
/* {{{ proto array glob(string pattern [, int flags]) U
   Find pathnames matching a pattern */
PHP_FUNCTION(glob)
{
	int cwd_skip = 0;
#ifdef ZTS
	char cwd[MAXPATHLEN];
	char work_pattern[MAXPATHLEN];
	char *result;
#endif
	zval **pppattern;
	char *pattern = NULL;
	int pattern_len;
	long flags = 0;
	glob_t globbuf;
	unsigned int n;
	int ret;
	zend_bool basedir_limit = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|l", &pppattern, &flags) == FAILURE ||
		php_stream_path_param_encode(pppattern, &pattern, &pattern_len, REPORT_ERRORS, FG(default_context)) == FAILURE) {
		return;
	}

	if (pattern_len >= MAXPATHLEN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Pattern exceeds the maximum allowed length of %d characters", MAXPATHLEN);
		RETURN_FALSE;
	}

	if ((GLOB_AVAILABLE_FLAGS & flags) != flags) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "At least one of the passed flags is invalid or not supported on this platform");
		RETURN_FALSE;
	}

#ifdef ZTS 
	if (!IS_ABSOLUTE_PATH(pattern, pattern_len)) {
		result = VCWD_GETCWD(cwd, MAXPATHLEN);	
		if (!result) {
			cwd[0] = '\0';
		}
#ifdef PHP_WIN32
		if (IS_SLASH(*pattern)) {
			cwd[2] = '\0';
		}
#endif
		cwd_skip = strlen(cwd)+1;

		snprintf(work_pattern, MAXPATHLEN, "%s%c%s", cwd, DEFAULT_SLASH, pattern);
		pattern = work_pattern;
	} 
#endif

	memset(&globbuf, 0, sizeof(glob_t));
	globbuf.gl_offs = 0;
	if (0 != (ret = glob(pattern, flags & GLOB_FLAGMASK, NULL, &globbuf))) {
#ifdef GLOB_NOMATCH
		if (GLOB_NOMATCH == ret) {
			/* Some glob implementation simply return no data if no matches
			   were found, others return the GLOB_NOMATCH error code.
			   We don't want to treat GLOB_NOMATCH as an error condition
			   so that PHP glob() behaves the same on both types of 
			   implementations and so that 'foreach (glob() as ...'
			   can be used for simple glob() calls without further error
			   checking.
			*/
			goto no_results;
		}
#endif
		RETURN_FALSE;
	}

	/* now catch the FreeBSD style of "no matches" */
	if (!globbuf.gl_pathc || !globbuf.gl_pathv) {
no_results:
		if (PG(open_basedir) && *PG(open_basedir)) {
			struct stat s;

			if (0 != VCWD_STAT(pattern, &s) || S_IFDIR != (s.st_mode & S_IFMT)) {
				RETURN_FALSE;
			}
		}
		array_init(return_value);
		return;
	}

	array_init(return_value);
	for (n = 0; n < globbuf.gl_pathc; n++) {
		if (PG(open_basedir) && *PG(open_basedir)) {
			if (php_check_open_basedir_ex(globbuf.gl_pathv[n], 0 TSRMLS_CC)) {
				basedir_limit = 1;
				continue;
			}
		}
		/* we need to do this everytime since GLOB_ONLYDIR does not guarantee that
		 * all directories will be filtered. GNU libc documentation states the
		 * following: 
		 * If the information about the type of the file is easily available 
		 * non-directories will be rejected but no extra work will be done to 
		 * determine the information for each file. I.e., the caller must still be 
		 * able to filter directories out. 
		 */
		if (flags & GLOB_ONLYDIR) {
			struct stat s;

			if (0 != VCWD_STAT(globbuf.gl_pathv[n], &s)) {
				continue;
			}

			if (S_IFDIR != (s.st_mode & S_IFMT)) {
				continue;
			}
		}
		if (UG(unicode)) {
			UChar *path;
			int path_len;

			if (SUCCESS == php_stream_path_decode(&php_plain_files_wrapper, &path, &path_len, globbuf.gl_pathv[n]+cwd_skip, 
								strlen(globbuf.gl_pathv[n]+cwd_skip), REPORT_ERRORS, FG(default_context))) {
				add_next_index_unicodel(return_value, path, path_len, 0);
			} else {
				/* Fallback on string version, path_decode will emit warning */
				add_next_index_string(return_value, globbuf.gl_pathv[n]+cwd_skip, 1);
			}
		} else {
			add_next_index_string(return_value, globbuf.gl_pathv[n]+cwd_skip, 1);
		}
	}

	globfree(&globbuf);

	if (basedir_limit && !zend_hash_num_elements(Z_ARRVAL_P(return_value))) {
		zval_dtor(return_value);
		RETURN_FALSE;
	}
}
/* }}} */
#endif 

/* {{{ proto array scandir(string dir [, int sorting_order [, resource context]]) U
   List files & directories inside the specified path */
PHP_FUNCTION(scandir)
{
	zval **ppdirn;
	char *dirn;
	int dirn_len;
	long flags = 0;
	char **namelist;
	int n, i;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|lr", &ppdirn, &flags, &zcontext) == FAILURE) {
		return;
	}

	context = php_stream_context_from_zval(zcontext, 0);
	if (FAILURE == php_stream_path_param_encode(ppdirn, &dirn, &dirn_len, REPORT_ERRORS, context)) {
		RETURN_FALSE;
	}

	if (!flags) {
		n = php_stream_scandir(dirn, &namelist, context, (void *) php_stream_dirent_alphasort);
	} else {
		n = php_stream_scandir(dirn, &namelist, context, (void *) php_stream_dirent_alphasortr);
	}
	if (n < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "(errno %d): %s", errno, strerror(errno));
		RETURN_FALSE;
	}
	
	array_init(return_value);

	for (i = 0; i < n; i++) {
		if (UG(unicode)) {
			UChar *path;
			int path_len;

			if (SUCCESS == php_stream_path_decode(NULL, &path, &path_len, namelist[i], strlen(namelist[i]), REPORT_ERRORS, context)) {
				add_next_index_unicodel(return_value, path, path_len, 0);
				efree(namelist[i]);
			} else {
				/* Fallback on using the non-unicode version, path_decode will emit the warning for us */
				add_next_index_string(return_value, namelist[i], 0);
			}
		} else {
			add_next_index_string(return_value, namelist[i], 0);
		}
	}

	if (n) {
		efree(namelist);
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
