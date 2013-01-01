/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Michael Maclean <mgdm@php.net>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_opencv.h"

zend_class_entry *opencv_ce_cvexception;

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_error)
{
	zend_class_entry ce, status_ce;

	INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Exception", NULL);
	opencv_ce_cvexception = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), "Exception" TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

PHP_OPENCV_API void php_opencv_throw_exception(TSRMLS_D)
{
	char * error_message;
	int status = cvGetErrStatus();

	if (status >= 0) {
		return;
	}

	error_message = estrdup(cvErrorStr(status));
	zend_throw_exception(opencv_ce_cvexception, error_message, status TSRMLS_CC);
	efree(error_message);
	return;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
