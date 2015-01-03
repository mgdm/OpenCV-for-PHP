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

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_opencv.h"

/* If you declare any globals in php_opencv.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(opencv)
*/

/* True global resources - no need for thread safety here */
static int le_opencv;

PHP_OPENCV_API void php_opencv_basedir_check(const char *filename TSRMLS_DC) {
	const char *error_message;
	int status;
	
#if PHP_VERSION_ID < 50400
	if (PG(safe_mode) && (!php_checkuid_ex(filename, NULL, CHECKUID_CHECK_FILE_AND_DIR, CHECKUID_NO_ERRORS))) {
        error_message = estrdup("Could not access file due to safe_mode restrictions");
        zend_throw_exception(opencv_ce_cvexception, error_message, status TSRMLS_CC);
        efree(error_message);
        return;
	}
#endif

	if (PG(open_basedir) && php_check_open_basedir_ex(filename, 0 TSRMLS_CC)) {
        error_message = estrdup("Could not access file due to open_basedir restrictions");
        zend_throw_exception(opencv_ce_cvexception, error_message, status TSRMLS_CC);
        efree(error_message);
		return;
	}
}

zend_class_entry *opencv_ce_cv;
/* {{{ proto void contruct()
   OpenCV CANNOT be extended in userspace, this will throw an exception on use */
PHP_METHOD(OpenCV, __construct)
{
	zend_throw_exception(opencv_ce_cvexception, "OpenCV cannot be constructed", 0 TSRMLS_CC);
}
/* }}} */

/* {{{ opencv_functions[]
 *
 * Every user visible function must have an entry in opencv_functions[].
 */
const zend_function_entry opencv_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in opencv_functions[] */
};
/* }}} */

/* {{{ opencv_module_entry
 */
zend_module_entry opencv_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"opencv",
	opencv_functions,
	PHP_MINIT(opencv),
	PHP_MSHUTDOWN(opencv),
	PHP_RINIT(opencv),
	NULL,
	PHP_MINFO(opencv),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_OPENCV
ZEND_GET_MODULE(opencv)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(opencv)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	PHP_MINIT(opencv_error)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(opencv_arr)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(opencv_mat)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(opencv_image)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(opencv_histogram)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(opencv_capture)(INIT_FUNC_ARGS_PASSTHRU);
	cvSetErrMode(CV_ErrModeSilent);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(opencv)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(opencv)
{
	cvSetErrMode(CV_ErrModeSilent);
	return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(opencv)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "OpenCV support", "Enabled");
	php_info_print_table_row(2, "OpenCV library version", CV_VERSION);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
