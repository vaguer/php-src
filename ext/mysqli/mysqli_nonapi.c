/*
  +----------------------------------------------------------------------+
  | PHP Version 6                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Georg Richter <georg@php.net>                               |
  |          Andrey Hristov <andrey@php.net>                             |
  |          Ulf Wendel <uw@php.net>                                     |
  +----------------------------------------------------------------------+

  $Id$ 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_mysqli_structs.h"

#define SAFE_STR(a) ((a)?a:"")

/* {{{ php_mysqli_set_error
 */
static void php_mysqli_set_error(long mysql_errno, char *mysql_err TSRMLS_DC)
{
	MyG(error_no) = mysql_errno;
	if (MyG(error_msg)) {
		efree(MyG(error_msg));
	}
	MyG(error_msg) = estrdup(mysql_err);
}
/* }}} */


void mysqli_common_connect(INTERNAL_FUNCTION_PARAMETERS, zend_bool is_real_connect, zend_bool in_ctor)
{
	MY_MYSQL			*mysql = NULL;
	MYSQLI_RESOURCE		*mysqli_resource = NULL;
	zval				*object = getThis();
	char				*hostname = NULL, *username=NULL, *passwd=NULL, *dbname=NULL, *socket=NULL;
	unsigned int		hostname_len = 0, username_len = 0, passwd_len = 0, dbname_len = 0, socket_len = 0;
	zend_bool			persistent = FALSE;
	long				port = 0, flags = 0;
	uint				hash_len;
	char				*hash_key = NULL;
	zend_bool			new_connection = FALSE;
	zend_rsrc_list_entry	*le;
	mysqli_plist_entry *plist = NULL;


#if !defined(MYSQL_USE_MYSQLND)
	if ((MYSQL_VERSION_ID / 100) != (mysql_get_client_version() / 100)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Headers and client library minor version mismatch. Headers:%d Library:%ld",
						MYSQL_VERSION_ID, mysql_get_client_version());
	}
#endif

	if (getThis() && !ZEND_NUM_ARGS() && in_ctor) {
		RETURN_NULL();
	}
	hostname = username = dbname = passwd = socket = NULL;

	if (!is_real_connect) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s&s&s&s&ls&", &hostname, &hostname_len, UG(utf8_conv),
								  &username, &username_len, UG(utf8_conv), &passwd, &passwd_len, UG(utf8_conv), 
								  &dbname, &dbname_len, UG(utf8_conv), &port, &socket, &socket_len, UG(utf8_conv)) == FAILURE) {
			return;
		}

		if (object && instanceof_function(Z_OBJCE_P(object), mysqli_link_class_entry TSRMLS_CC)) {
			mysqli_resource = ((mysqli_object *) zend_object_store_get_object(object TSRMLS_CC))->ptr;
			if (mysqli_resource && mysqli_resource->ptr) {
				mysql = (MY_MYSQL*) mysqli_resource->ptr;			
				if (mysqli_resource->status > MYSQLI_STATUS_INITIALIZED) {
					php_clear_mysql(mysql);
					if (mysql->mysql) {
						mysqli_close(mysql->mysql, MYSQLI_CLOSE_EXPLICIT);
						mysql->mysql = NULL;
					}
				}
			}
		}
		if (!mysql) {
			mysql = (MY_MYSQL *) ecalloc(1, sizeof(MY_MYSQL));
		}
		flags |= CLIENT_MULTI_RESULTS; /* needed for mysql_multi_query() */
	} else {
		/* We have flags too */
		if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|s&s&s&s&ls&l", &object, mysqli_link_class_entry,
			&hostname, &hostname_len, UG(utf8_conv), &username, &username_len, UG(utf8_conv), &passwd, &passwd_len, UG(utf8_conv),
			&dbname, &dbname_len, UG(utf8_conv), &port, &socket, &socket_len, UG(utf8_conv), &flags) == FAILURE) {
			return;
		}

		mysqli_resource = ((mysqli_object *) zend_object_store_get_object(object TSRMLS_CC))->ptr;
		MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL *, &object, "mysqli_link", MYSQLI_STATUS_INITIALIZED);

		/* set some required options */
		flags |= CLIENT_MULTI_RESULTS; /* needed for mysql_multi_query() */
		/* remove some insecure options */
		flags &= ~CLIENT_MULTI_STATEMENTS;   /* don't allow multi_queries via connect parameter */
		if (PG(open_basedir) && PG(open_basedir)[0] != '\0') {
			flags &= ~CLIENT_LOCAL_FILES;
		}
	}


	if (!socket_len || !socket) {
		socket = MyG(default_socket);
	}
	if (!port){
		port = MyG(default_port);
	}
	if (!passwd) {
		passwd = MyG(default_pw);
		passwd_len = strlen(SAFE_STR(passwd));
	}
	if (!username){
		username = MyG(default_user);
	}
	if (!hostname || !hostname_len) {
		hostname = MyG(default_host);
	}

	if (strlen(SAFE_STR(hostname)) > 2 && !strncasecmp(hostname, "p:", 2)) {
		hostname += 2;
		if (!MyG(allow_persistent)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Persistent connections are disabled. Downgrading to normal");			
		} else {
			mysql->persistent = persistent = TRUE;

			hash_len = spprintf(&hash_key, 0, "mysqli_%s_%s%ld%s%s%s", SAFE_STR(hostname), SAFE_STR(socket), 
								port, SAFE_STR(username), SAFE_STR(dbname), 
								SAFE_STR(passwd));

			mysql->hash_key = hash_key;

			/* check if we can reuse exisiting connection ... */
			if (zend_hash_find(&EG(persistent_list), hash_key, hash_len + 1, (void **)&le) == SUCCESS) {
				if (Z_TYPE_P(le) == php_le_pmysqli()) {
					plist = (mysqli_plist_entry *) le->ptr;

					do {
						if (zend_ptr_stack_num_elements(&plist->free_links)) {
							if (is_real_connect) {
								/*
								  Gotcha! If there are some options set on the handle with mysqli_options()
								  they will be lost. We will fetch other handle with other options. This could
								  be a source of bug reports of people complaining but...nothing else could be
								  done, if they want PCONN!
								*/
								mysqli_close(mysql->mysql, MYSQLI_CLOSE_IMPLICIT);
							}
							mysql->mysql = zend_ptr_stack_pop(&plist->free_links);

							MyG(num_inactive_persistent)--;
#if defined(MYSQLI_USE_MYSQLND)
							mysqlnd_end_psession(mysql->mysql);
#endif	
							/* reset variables */
							/* todo: option for ping or change_user */

#if G0
							if (!mysql_change_user(mysql->mysql, username, passwd, dbname)) {
#else
							if (!mysql_ping(mysql->mysql)) {
#endif
#ifdef MYSQLI_USE_MYSQLND
								mysqlnd_restart_psession(mysql->mysql, MyG(mysqlnd_thd_zval_cache));
#endif
								MyG(num_active_persistent)++;
								goto end;
							} else {
#if defined(MYSQLI_USE_MYSQLND)
								mysqlnd_end_psession(mysql->mysql);
#endif	
								mysqli_close(mysql->mysql, MYSQLI_CLOSE_IMPLICIT);
								mysql->mysql = NULL;
							}
						}
					} while (0);
				}
			} else {
				zend_rsrc_list_entry le;
				le.type = php_le_pmysqli();
				le.ptr = plist = calloc(1, sizeof(mysqli_plist_entry));

				zend_ptr_stack_init_ex(&plist->free_links, 1);
				zend_hash_update(&EG(persistent_list), hash_key, hash_len + 1, (void *)&le, sizeof(le), NULL);
			}
		}
	}

	if (MyG(max_links) != -1 && MyG(num_links) >= MyG(max_links)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Too many open links (%ld)", MyG(num_links));
		goto err;
	}
	if (persistent && MyG(max_persistent) != -1 &&
		(MyG(num_active_persistent) + MyG(num_inactive_persistent))>= MyG(max_persistent))
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Too many open persistent links (%ld)",
								MyG(num_active_persistent) + MyG(num_inactive_persistent));
		goto err;
	}
	if (!is_real_connect && !mysql->mysql) {
#if !defined(MYSQLI_USE_MYSQLND)
		if (!(mysql->mysql = mysql_init(NULL))) {
#else
		if (!(mysql->mysql = mysqlnd_init(persistent))) {
#endif
			goto err;
		}
		new_connection = TRUE;
	}

	if (UG(unicode)) {
		mysql_options(mysql->mysql, MYSQL_SET_CHARSET_NAME, "utf8");
	}


#ifdef HAVE_EMBEDDED_MYSQLI
	if (hostname_len) {
		unsigned int external=1;
		mysql_options(mysql->mysql, MYSQL_OPT_USE_REMOTE_CONNECTION, (char *)&external);
	} else {
		mysql_options(mysql->mysql, MYSQL_OPT_USE_EMBEDDED_CONNECTION, 0);
	}
#endif

#if !defined(MYSQLI_USE_MYSQLND)
	if (mysql_real_connect(mysql->mysql, hostname, username, passwd, dbname, port, socket, CLIENT_MULTI_RESULTS) == NULL)
#else
	if (mysqlnd_connect(mysql->mysql, hostname, username, passwd, passwd_len, dbname, dbname_len,
						port, socket, flags, MyG(mysqlnd_thd_zval_cache) TSRMLS_CC) == NULL)
#endif
	{
		/* Save error messages - for mysqli_connect_error() & mysqli_connect_errno() */
		php_mysqli_set_error(mysql_errno(mysql->mysql), (char *) mysql_error(mysql->mysql) TSRMLS_CC);
		php_mysqli_throw_sql_exception((char *)mysql_sqlstate(mysql->mysql), mysql_errno(mysql->mysql) TSRMLS_CC,
										"%s", mysql_error(mysql->mysql));
		if (!is_real_connect) {
			/* free mysql structure */
			mysqli_close(mysql->mysql, MYSQLI_CLOSE_DISCONNECTED);
		}
		goto err;
	}

	/* when PHP runs in unicode, set default character set to utf8 */
	if (UG(unicode)) {
		mysql->conv = UG(utf8_conv);
	}

	/* clear error */
	php_mysqli_set_error(mysql_errno(mysql->mysql), (char *) mysql_error(mysql->mysql) TSRMLS_CC);

#if !defined(MYSQLI_USE_MYSQLND)
	mysql->mysql->reconnect = MyG(reconnect);

	/* set our own local_infile handler */
	php_set_local_infile_handler_default(mysql);
#endif

	mysql_options(mysql->mysql, MYSQL_OPT_LOCAL_INFILE, (char *)&MyG(allow_local_infile));

end:
	if (!mysqli_resource) {
		mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
		mysqli_resource->ptr = (void *)mysql;
	}
	mysqli_resource->status = MYSQLI_STATUS_VALID;

	/* store persistent connection */
	if (persistent && (new_connection || is_real_connect)) {
		MyG(num_active_persistent)++;
	}

	MyG(num_links)++;

	mysql->multi_query = 0;

	if (!object || !instanceof_function(Z_OBJCE_P(object), mysqli_link_class_entry TSRMLS_CC)) {
		MYSQLI_RETURN_RESOURCE(mysqli_resource, mysqli_link_class_entry);	
	} else {
		((mysqli_object *) zend_object_store_get_object(object TSRMLS_CC))->ptr = mysqli_resource;
	}
	if (!is_real_connect) {
		return;
	} else {
		RETURN_TRUE;
	}

err:
	if (mysql->hash_key) {
		efree(mysql->hash_key);
		mysql->hash_key = NULL;
	}
	if (!is_real_connect) {
		efree(mysql);
	}
	RETVAL_FALSE;
}


/* {{{ proto object mysqli_connect([string hostname [,string username [,string passwd [,string dbname [,int port [,string socket]]]]]])
   Open a connection to a mysql server */ 
PHP_FUNCTION(mysqli_connect)
{
	mysqli_common_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, FALSE);
}
/* }}} */


/* {{{ proto object mysqli_link_construct()
  */ 
PHP_FUNCTION(mysqli_link_construct)
{
	mysqli_common_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, TRUE);
}
/* }}} */


/* {{{ proto int mysqli_connect_errno(void) U
   Returns the numerical value of the error message from last connect command */
PHP_FUNCTION(mysqli_connect_errno)
{
	RETURN_LONG(MyG(error_no));
}
/* }}} */

/* {{{ proto string mysqli_connect_error(void) U
   Returns the text of the error message from previous MySQL operation */
PHP_FUNCTION(mysqli_connect_error) 
{
	if (MyG(error_msg)) {
		RETURN_UTF8_STRING((char *)MyG(error_msg), ZSTR_DUPLICATE);
	} else {
		RETURN_NULL();
	}
}
/* }}} */


/* {{{ proto mixed mysqli_fetch_array (object result [,int resulttype]) U
   Fetch a result row as an associative array, a numeric array, or both */
PHP_FUNCTION(mysqli_fetch_array) 
{
#if !defined(MYSQLI_USE_MYSQLND)
	php_mysqli_fetch_into_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
#else
	MYSQL_RES		*result;
	zval			*mysql_result;
	long			mode = MYSQLND_FETCH_BOTH;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &mysql_result, mysqli_result_class_entry, &mode) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, "mysqli_result", MYSQLI_STATUS_VALID);

	if (mode < MYSQLI_ASSOC || mode > MYSQLI_BOTH) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The result type should be either MYSQLI_NUM, MYSQLI_ASSOC or MYSQLI_BOTH");
		RETURN_FALSE;
	}

	mysqlnd_fetch_into(result, mode, return_value, MYSQLND_MYSQLI);
#endif
}
/* }}} */

/* {{{ proto mixed mysqli_fetch_assoc (object result) U
   Fetch a result row as an associative array */
PHP_FUNCTION(mysqli_fetch_assoc) 
{
#if !defined(MYSQLI_USE_MYSQLND)
	php_mysqli_fetch_into_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQLI_ASSOC, 0);
#else
	MYSQL_RES		*result;
	zval			*mysql_result;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &mysql_result, mysqli_result_class_entry) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, "mysqli_result", MYSQLI_STATUS_VALID);
	mysqlnd_fetch_into(result, MYSQLND_FETCH_ASSOC, return_value, MYSQLND_MYSQLI);
	
#endif
}
/* }}} */


/* {{{ proto mixed mysqli_fetch_all (object result [,int resulttype]) U
   Fetches all result rows as an associative array, a numeric array, or both */
#if defined(MYSQLI_USE_MYSQLND)
PHP_FUNCTION(mysqli_fetch_all) 
{
	MYSQL_RES		*result;
	zval			*mysql_result;
	long			mode = MYSQLND_FETCH_NUM;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &mysql_result, mysqli_result_class_entry, &mode) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, "mysqli_result", MYSQLI_STATUS_VALID);

	if (!mode || (mode & ~MYSQLND_FETCH_BOTH)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Mode can be only MYSQLI_FETCH_NUM, "
		                 "MYSQLI_FETCH_ASSOC or MYSQLI_FETCH_BOTH");
		RETURN_FALSE;
	}

	mysqlnd_fetch_all(result, mode, return_value);
}
/* }}} */


/* {{{ proto array mysqli_cache_stats(void) U
   Returns statistics about the zval cache */
PHP_FUNCTION(mysqli_get_cache_stats) 
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	mysqlnd_palloc_stats(mysqli_mysqlnd_zval_cache, return_value);
}
/* }}} */


/* {{{ proto array mysqli_get_client_stats(void)
   Returns statistics about the zval cache */
PHP_FUNCTION(mysqli_get_client_stats) 
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	mysqlnd_get_client_stats(return_value);
}
/* }}} */


/* {{{ proto array mysqli_get_connection_stats(void)
   Returns statistics about the zval cache */
PHP_FUNCTION(mysqli_get_connection_stats) 
{
	MY_MYSQL	*mysql;
	zval		*mysql_link;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
									 &mysql_link, mysqli_link_class_entry) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL *, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	mysqlnd_get_connection_stats(mysql->mysql, return_value);
}
#endif
/* }}} */


/* {{{ proto mixed mysqli_fetch_object (object result [, string class_name [, NULL|array ctor_params]]) U
   Fetch a result row as an object */
PHP_FUNCTION(mysqli_fetch_object) 
{
	php_mysqli_fetch_into_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQLI_ASSOC, 1);	
/* todo: mysqlnd support */
}
/* }}} */

/* {{{ proto bool mysqli_multi_query(object link, string query) U
   allows to execute multiple queries  */
PHP_FUNCTION(mysqli_multi_query)
{
	MY_MYSQL		*mysql;
	zval			*mysql_link;
	char			*query = NULL;
	unsigned int 	query_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os&", &mysql_link, mysqli_link_class_entry, 
									 &query, &query_len, UG(utf8_conv)) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL *, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	MYSQLI_ENABLE_MQ;	
	if (mysql_real_query(mysql->mysql, query, query_len)) {
#ifndef MYSQLI_USE_MYSQLND
		char s_error[MYSQL_ERRMSG_SIZE], s_sqlstate[SQLSTATE_LENGTH+1];
		unsigned int s_errno;
		/* we have to save error information, cause 
		MYSQLI_DISABLE_MQ will reset error information */
		strcpy(s_error, mysql_error(mysql->mysql));
		strcpy(s_sqlstate, mysql_sqlstate(mysql->mysql));
		s_errno = mysql_errno(mysql->mysql);
#else
		mysqlnd_error_info error_info = mysql->mysql->error_info;
#endif
		MYSQLI_REPORT_MYSQL_ERROR(mysql->mysql);
		MYSQLI_DISABLE_MQ;

#ifndef MYSQLI_USE_MYSQLND
		/* restore error information */
		strcpy(mysql->mysql->net.last_error, s_error);
		strcpy(mysql->mysql->net.sqlstate, s_sqlstate);
		mysql->mysql->net.last_errno = s_errno;	
#else
		mysql->mysql->error_info = error_info;
#endif
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed mysqli_query(object link, string query [,int resultmode]) U */
PHP_FUNCTION(mysqli_query)
{
	MY_MYSQL			*mysql;
	zval				*mysql_link;
	MYSQLI_RESOURCE		*mysqli_resource;
	MYSQL_RES 			*result;
	char				*query = NULL;
	unsigned int 		query_len;
	unsigned long 		resultmode = MYSQLI_STORE_RESULT;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os&|l", &mysql_link, mysqli_link_class_entry, 
									 &query, &query_len, UG(utf8_conv), &resultmode) == FAILURE) {
		return;
	}

	if (!query_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty query");
		RETURN_FALSE;
	}
	if ((resultmode & ~MYSQLI_ASYNC) != MYSQLI_USE_RESULT && (resultmode & ~MYSQLI_ASYNC) != MYSQLI_STORE_RESULT
#if defined(MYSQLI_USE_MYSQLND) && defined(MYSQLND_THREADED)
		&& (resultmode & ~MYSQLI_ASYNC) != MYSQLI_BG_STORE_RESULT
#endif
	) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value for resultmode");
		RETURN_FALSE;
	}

	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL*, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	MYSQLI_DISABLE_MQ;


#ifdef MYSQLI_USE_MYSQLND
	if (resultmode & MYSQLI_ASYNC) {
		if (mysqli_async_query(mysql->mysql, query, query_len)) {
			MYSQLI_REPORT_MYSQL_ERROR(mysql->mysql);
			RETURN_FALSE;
		}
		mysql->async_result_fetch_type = resultmode & ~MYSQLI_ASYNC;
		RETURN_TRUE;
	}
#endif

	if (mysql_real_query(mysql->mysql, query, query_len)) {
		MYSQLI_REPORT_MYSQL_ERROR(mysql->mysql);
		RETURN_FALSE;
	}

	if (!mysql_field_count(mysql->mysql)) {
		/* no result set - not a SELECT */
		if (MyG(report_mode) & MYSQLI_REPORT_INDEX) {
			php_mysqli_report_index(query, mysqli_server_status(mysql->mysql) TSRMLS_CC);
		}
		RETURN_TRUE;
	}

	switch (resultmode) {
		case MYSQLI_STORE_RESULT:
			result = mysql_store_result(mysql->mysql);
			break;
		case MYSQLI_USE_RESULT:
			result = mysql_use_result(mysql->mysql);
			break;
#if defined(MYSQLI_USE_MYSQLND) && defined(MYSQLND_THREADED)
		case MYSQLI_BG_STORE_RESULT:
			result = mysqli_bg_store_result(mysql->mysql);
			break;
#endif
	}
	if (!result) {
		php_mysqli_throw_sql_exception((char *)mysql_sqlstate(mysql->mysql), mysql_errno(mysql->mysql) TSRMLS_CC,
										"%s", mysql_error(mysql->mysql)); 
		RETURN_FALSE;
	}

	if (MyG(report_mode) & MYSQLI_REPORT_INDEX) {
		php_mysqli_report_index(query, mysqli_server_status(mysql->mysql) TSRMLS_CC);
	}

	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = (void *)result;
	mysqli_resource->status = MYSQLI_STATUS_VALID;
	MYSQLI_RETURN_RESOURCE(mysqli_resource, mysqli_result_class_entry);
}
/* }}} */


#if defined(MYSQLI_USE_MYSQLND)
#include "php_network.h"
/* {{{ mysqlnd_zval_array_to_mysqlnd_array functions */
static int mysqlnd_zval_array_to_mysqlnd_array(zval *in_array, MYSQLND ***out_array TSRMLS_DC)
{
	zval **elem;
	int i = 0, current = 0;

	if (Z_TYPE_P(in_array) != IS_ARRAY) {
		return 0;
	}
	*out_array = ecalloc(zend_hash_num_elements(Z_ARRVAL_P(in_array)) + 1, sizeof(MYSQLND *));
	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(in_array));
		 zend_hash_get_current_data(Z_ARRVAL_P(in_array), (void **) &elem) == SUCCESS;
		 zend_hash_move_forward(Z_ARRVAL_P(in_array))) {
		i++;
		if (Z_TYPE_PP(elem) != IS_OBJECT ||
			!instanceof_function(Z_OBJCE_PP(elem), mysqli_link_class_entry TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameter %d not a mysqli object", i);			
		} else {
			MY_MYSQL *mysql;
			MYSQLI_RESOURCE *my_res;
			mysqli_object *intern = (mysqli_object *)zend_object_store_get_object(*elem TSRMLS_CC);
			if (!(my_res = (MYSQLI_RESOURCE *)intern->ptr)) {
		  		php_error_docref(NULL TSRMLS_CC, E_WARNING, "[%d] Couldn't fetch %v", i, intern->zo.ce->name);
				continue;
		  	}
			mysql = (MY_MYSQL*) my_res->ptr;
			if (MYSQLI_STATUS_VALID && my_res->status < MYSQLI_STATUS_VALID) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid object %d or resource %v", i, intern->zo.ce->name);
				continue;
			}
			(*out_array)[current++] = mysql->mysql;
		}
	}
	return 0;
}
/* }}} */


/* {{{ mysqlnd_zval_array_from_mysqlnd_array */
static int mysqlnd_zval_array_from_mysqlnd_array(MYSQLND **in_array, zval *out_array TSRMLS_DC)
{
	MYSQLND **p = in_array;
	HashTable *new_hash;
	zval **elem, **dest_elem;
	int ret = 0;

	ALLOC_HASHTABLE(new_hash);
	zend_hash_init(new_hash, zend_hash_num_elements(Z_ARRVAL_P(out_array)), NULL, ZVAL_PTR_DTOR, 0);

	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(out_array));
		 zend_hash_get_current_data(Z_ARRVAL_P(out_array), (void **) &elem) == SUCCESS;
		 zend_hash_move_forward(Z_ARRVAL_P(out_array)))
	{
		if (Z_TYPE_PP(elem) != IS_OBJECT || !instanceof_function(Z_OBJCE_PP(elem), mysqli_link_class_entry TSRMLS_CC)) {
			continue;
		}
		{
			MY_MYSQL *mysql;
			mysqli_object *intern = (mysqli_object *)zend_object_store_get_object(*elem TSRMLS_CC);
			mysql = (MY_MYSQL *) ((MYSQLI_RESOURCE *)intern->ptr)->ptr;
			if (mysql->mysql == *p) {
				zend_hash_next_index_insert(new_hash, (void *)elem, sizeof(zval *), (void **)&dest_elem);
				if (dest_elem) {
					zval_add_ref(dest_elem);
				}
				ret++;
				p++;
			}
		}
	}

	/* destroy old array and add new one */
	zend_hash_destroy(Z_ARRVAL_P(out_array));
	efree(Z_ARRVAL_P(out_array));

	zend_hash_internal_pointer_reset(new_hash);
	Z_ARRVAL_P(out_array) = new_hash;

	return 0;
}
/* }}} */


/* {{{ mysqlnd_dont_poll_zval_array_from_mysqlnd_array */
static int mysqlnd_dont_poll_zval_array_from_mysqlnd_array(MYSQLND **in_array, zval *in_zval_array, zval *out_array TSRMLS_DC)
{
	MYSQLND **p = in_array;
	HashTable *new_hash;
	zval **elem, **dest_elem;
	int ret = 0;

	ALLOC_HASHTABLE(new_hash);
	zend_hash_init(new_hash, zend_hash_num_elements(Z_ARRVAL_P(in_zval_array)), NULL, ZVAL_PTR_DTOR, 0);
	if (in_array) {
		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(in_zval_array));
			 zend_hash_get_current_data(Z_ARRVAL_P(in_zval_array), (void **) &elem) == SUCCESS;
			 zend_hash_move_forward(Z_ARRVAL_P(in_zval_array)))
		{
			MY_MYSQL *mysql;
			mysqli_object *intern = (mysqli_object *)zend_object_store_get_object(*elem TSRMLS_CC);
			mysql = (MY_MYSQL *) ((MYSQLI_RESOURCE *)intern->ptr)->ptr;
			if (mysql->mysql == *p) {
				zend_hash_next_index_insert(new_hash, (void *)elem, sizeof(zval *), (void **)&dest_elem);
				if (dest_elem) {
					zval_add_ref(dest_elem);
				}
				ret++;
				p++;
			}
		}
	}

	/* destroy old array and add new one */
	zend_hash_destroy(Z_ARRVAL_P(out_array));
	efree(Z_ARRVAL_P(out_array));

	zend_hash_internal_pointer_reset(new_hash);
	Z_ARRVAL_P(out_array) = new_hash;

	return 0;
}
/* }}} */


/* {{{ proto int mysqli_poll(array read, array write, array error, long sec [, long usec]) U
   Poll connections */
PHP_FUNCTION(mysqli_poll)
{
	zval			*r_array, *e_array, *dont_poll_array;
	MYSQLND			**new_r_array = NULL, **new_e_array = NULL, **new_dont_poll_array = NULL;
	long			sec = 0, usec = 0;
	enum_func_status ret;
	uint 			desc_num;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a!a!al|l", &r_array, &e_array, &dont_poll_array, &sec, &usec) == FAILURE) {
		return;
	}
	if (sec < 0 || usec < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Negative values passed for sec and/or usec");
		RETURN_FALSE;
	}

	if (r_array != NULL) {
		mysqlnd_zval_array_to_mysqlnd_array(r_array, &new_r_array TSRMLS_CC);
	}
	if (e_array != NULL) {
		mysqlnd_zval_array_to_mysqlnd_array(e_array, &new_e_array TSRMLS_CC);
	}

	ret = mysqlnd_poll(new_r_array, new_e_array, &new_dont_poll_array, sec, usec, &desc_num);

	mysqlnd_dont_poll_zval_array_from_mysqlnd_array(r_array != NULL ? new_dont_poll_array:NULL, r_array, dont_poll_array TSRMLS_CC);

	if (r_array != NULL) {
		mysqlnd_zval_array_from_mysqlnd_array(new_r_array, r_array TSRMLS_CC);
	}
	if (e_array != NULL) {
		mysqlnd_zval_array_from_mysqlnd_array(new_e_array, e_array TSRMLS_CC);
	}

	if (new_dont_poll_array) {
		efree(new_dont_poll_array);
	}
	if (new_r_array) {
		efree(new_r_array);
	}
	if (new_e_array) {
		efree(new_e_array);
	}
	if (ret == PASS) {
		RETURN_LONG(desc_num);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ proto int mysqli_reap_async_query(object link) U
   Poll connections */
PHP_FUNCTION(mysqli_reap_async_query)
{
	MY_MYSQL		*mysql;
	zval			*mysql_link;
	MYSQLI_RESOURCE		*mysqli_resource;
	MYSQL_RES 			*result;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &mysql_link, mysqli_link_class_entry) == FAILURE) {
		return;
	}

	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL*, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	if (FAIL == mysqlnd_reap_async_query(mysql->mysql)) {
		RETURN_FALSE;
	}

	if (!mysql_field_count(mysql->mysql)) {
		/* no result set - not a SELECT */
		if (MyG(report_mode) & MYSQLI_REPORT_INDEX) {
/*			php_mysqli_report_index("n/a", mysqli_server_status(mysql->mysql) TSRMLS_CC); */
		}
		RETURN_TRUE;
	}

	switch (mysql->async_result_fetch_type) {
		case MYSQLI_STORE_RESULT:
			result = mysql_store_result(mysql->mysql);
			break;
		case MYSQLI_USE_RESULT:
			result = mysql_use_result(mysql->mysql);
			break;
#if defined(MYSQLI_USE_MYSQLND) && defined(MYSQLND_THREADED)
		case MYSQLI_BG_STORE_RESULT:
			result = mysqli_bg_store_result(mysql->mysql);
			break;
#endif
	}

	if (!result) {
		php_mysqli_throw_sql_exception((char *)mysql_sqlstate(mysql->mysql), mysql_errno(mysql->mysql) TSRMLS_CC,
										"%s", mysql_error(mysql->mysql)); 
		RETURN_FALSE;
	}

	if (MyG(report_mode) & MYSQLI_REPORT_INDEX) {
/*		php_mysqli_report_index("n/a", mysqli_server_status(mysql->mysql) TSRMLS_CC);*/
	}

	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = (void *)result;
	mysqli_resource->status = MYSQLI_STATUS_VALID;
	MYSQLI_RETURN_RESOURCE(mysqli_resource, mysqli_result_class_entry);
}
/* }}} */


/* {{{ proto object mysqli_stmt_get_result(object link) U
   Buffer result set on client */
PHP_FUNCTION(mysqli_stmt_get_result)
{
	MYSQL_RES 		*result;
	MYSQLI_RESOURCE	*mysqli_resource;
	MY_STMT			*stmt;
	zval 			*mysql_stmt;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &mysql_stmt, mysqli_stmt_class_entry) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(stmt, MY_STMT *, &mysql_stmt, "mysqli_stmt", MYSQLI_STATUS_VALID);

	if (!(result = mysqlnd_stmt_get_result(stmt->stmt))) {
		MYSQLI_REPORT_STMT_ERROR(stmt->stmt);
		RETURN_FALSE;
	}

	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = (void *)result;
	mysqli_resource->status = MYSQLI_STATUS_VALID;
	MYSQLI_RETURN_RESOURCE(mysqli_resource, mysqli_result_class_entry);	
}
/* }}} */
#endif


/* {{{ proto object mysqli_get_warnings(object link) U */
PHP_FUNCTION(mysqli_get_warnings)
{
	MY_MYSQL			*mysql;
	zval				*mysql_link;
	MYSQLI_RESOURCE		*mysqli_resource;
	MYSQLI_WARNING		*w;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &mysql_link, mysqli_link_class_entry) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL*, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	if (mysql_warning_count(mysql->mysql)) {
		w = php_get_warnings(mysql->mysql TSRMLS_CC); 
	} else {
		RETURN_FALSE;
	}
	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = mysqli_resource->info = (void *)w;
	mysqli_resource->status = MYSQLI_STATUS_VALID;
	MYSQLI_RETURN_RESOURCE(mysqli_resource, mysqli_warning_class_entry);	
}
/* }}} */

/* {{{ proto object mysqli_stmt_get_warnings(object link) U */
PHP_FUNCTION(mysqli_stmt_get_warnings)
{
	MY_STMT				*stmt;
	zval				*stmt_link;
	MYSQLI_RESOURCE		*mysqli_resource;
	MYSQLI_WARNING		*w;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &stmt_link, mysqli_stmt_class_entry) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(stmt, MY_STMT*, &stmt_link, "mysqli_stmt", MYSQLI_STATUS_VALID);

	if (mysqli_stmt_warning_count(stmt->stmt)) {
		w = php_get_warnings(mysqli_stmt_get_connection(stmt->stmt) TSRMLS_CC); 
	} else {
		RETURN_FALSE;
	}
	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = mysqli_resource->info = (void *)w;
	mysqli_resource->status = MYSQLI_STATUS_VALID;
	MYSQLI_RETURN_RESOURCE(mysqli_resource, mysqli_warning_class_entry);
}
/* }}} */

#ifdef HAVE_MYSQLI_SET_CHARSET
/* {{{ proto bool mysqli_set_charset(object link, string csname) U
   sets client character set */
PHP_FUNCTION(mysqli_set_charset)
{
	MY_MYSQL			*mysql;
	zval				*mysql_link;
	char				*cs_name;
	int					csname_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os&", &mysql_link, mysqli_link_class_entry, 
									 &cs_name, &csname_len, UG(utf8_conv)) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL*, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	/* check unicode modus */
	/* todo: we need also to support UCS2. This will not work when using SET NAMES */
	if (UG(unicode) && (csname_len != 4  || strncasecmp(cs_name, "utf8", 4))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Character set %s is not supported when running PHP with unicode.semantics=On.", cs_name);
		RETURN_FALSE;
	}

	if (mysql_set_character_set(mysql->mysql, cs_name)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */
#endif

#ifdef HAVE_MYSQLI_GET_CHARSET 
/* {{{ proto object mysqli_get_charset(object link) U
   returns a character set object */
PHP_FUNCTION(mysqli_get_charset)
{
	MY_MYSQL				*mysql;
	zval					*mysql_link;
	const char 				*name = NULL, *collation = NULL, *dir = NULL, *comment = NULL;
	uint					minlength, maxlength, number, state;
#if !defined(MYSQLI_USE_MYSQLND)
	MY_CHARSET_INFO			cs;
#else
	const MYSQLND_CHARSET	*cs;
#endif

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &mysql_link, mysqli_link_class_entry) == FAILURE) {
		return;
	}
	MYSQLI_FETCH_RESOURCE(mysql, MY_MYSQL*, &mysql_link, "mysqli_link", MYSQLI_STATUS_VALID);

	object_init(return_value);

#if !defined(MYSQLI_USE_MYSQLND)
	mysql_get_character_set_info(mysql->mysql, &cs);
	name = (char *)cs.csname;
	collation = (char *)cs.name;
	dir = (char *)cs.dir;
	minlength = cs.mbminlen;
	maxlength = cs.mbmaxlen;
	number = cs.number;
	state = cs.state;
	comment = cs.comment;
#else
	cs = mysql->mysql->charset;
	name = cs->name;	
	collation = cs->collation;	
	minlength = cs->char_minlen;
	maxlength = cs->char_maxlen;
	number = cs->nr;
	comment = cs->comment;
	state = 1;	/* all charsets are compiled in */
#endif

	if (UG(unicode)) {
		add_property_utf8_string(return_value, "charset", (name) ? (char *)name : "", 1);
		add_property_utf8_string(return_value, "collation", (collation) ? (char *)collation : "", 1);
		add_property_utf8_string(return_value, "dir", (dir) ? (char *)dir : "", 1);
	} else {
		add_property_string(return_value, "charset", (name) ? (char *)name : "", 1);
		add_property_string(return_value, "collation",(collation) ? (char *)collation : "", 1);
		add_property_string(return_value, "dir", (dir) ? (char *)dir : "", 1);
	}
	add_property_long(return_value, "min_length", minlength);
	add_property_long(return_value, "max_length", maxlength);
	add_property_long(return_value, "number", number);
	add_property_long(return_value, "state", state);
	if (UG(unicode)) {
		add_property_utf8_string(return_value, "comment", (comment) ? (char *)comment : "", 1);
	} else {
		add_property_string(return_value, "comment", (comment) ? (char *)comment : "", 1);
	}
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
