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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifndef PHP_OPENCV_H
#define PHP_OPENCV_H

extern zend_module_entry opencv_module_entry;
#define phpext_opencv_ptr &opencv_module_entry

#ifdef PHP_WIN32
#	define PHP_OPENCV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_OPENCV_API __attribute__ ((visibility("default")))
#else
#	define PHP_OPENCV_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(opencv);
PHP_MSHUTDOWN_FUNCTION(opencv);
PHP_RINIT_FUNCTION(opencv);
PHP_RSHUTDOWN_FUNCTION(opencv);
PHP_MINFO_FUNCTION(opencv);

PHP_FUNCTION(confirm_opencv_compiled);	/* For testing, remove later. */

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(opencv)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(opencv)
*/

/* In every utility function you add that needs to use variables 
   in php_opencv_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as OPENCV_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define OPENCV_G(v) TSRMG(opencv_globals_id, zend_opencv_globals *, v)
#else
#define OPENCV_G(v) (opencv_globals.v)
#endif

#endif	/* PHP_OPENCV_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
