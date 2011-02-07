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
#include "php_opencv.h"
#include "zend_exceptions.h"

zend_class_entry *opencv_ce_cvarr;

/* {{{ proto void contruct()
   CairoSurface CANNOT be extended in userspace, this will throw an exception on use */
PHP_METHOD(OpenCV_Arr, __construct)
{
	zend_throw_exception(opencv_ce_cvexception, "OpenCV\\Arr cannot be constructed", 0 TSRMLS_CC);
}
/* }}} */

/* {{{ opencv_arr_methods[] */
const zend_function_entry opencv_arr_methods[] = { 
    PHP_ME(OpenCV_Arr, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_arr)
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Arr", opencv_arr_methods);
	opencv_ce_cvarr = zend_register_internal_class(&ce TSRMLS_CC);

	return SUCCESS;
}
/* }}} */


