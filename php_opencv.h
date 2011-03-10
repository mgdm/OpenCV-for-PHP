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

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

/* Macros for PHP 5.2 */
#ifndef zend_parse_parameters_none
#define zend_parse_parameters_none()                                        \
    zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")
#endif

/* Constants for open_basedir checks */
#define OPENCV_READ_WRITE_NO_ERROR 0
#define OPENCV_READ_WRITE_SAFE_MODE_ERROR 1
#define OPENCV_READ_WRITE_OPEN_BASEDIR_ERROR 2

/* refcount macros */
#ifndef Z_ADDREF_P
#define Z_ADDREF_P(pz)                (pz)->refcount++
#endif

#ifndef Z_DELREF_P
#define Z_DELREF_P(pz)                (pz)->refcount--
#endif

#ifndef Z_SET_REFCOUNT_P
#define Z_SET_REFCOUNT_P(pz, rc)      (pz)->refcount = rc
#endif

/* turn error handling to exception mode and restore */
#if PHP_VERSION_ID >= 50300
/* 5.3 version of the macros */
#define PHP_OPENCV_ERROR_HANDLING() \
	zend_replace_error_handling(EH_THROW, opencv_ce_cvexception, &opencv_original_error_handling TSRMLS_CC)

#define PHP_OPENCV_RESTORE_ERRORS() \
	zend_restore_error_handling(&opencv_original_error_handling TSRMLS_CC)

#else
/* 5.2 versions of the macros */
#define PHP_OPENCV_ERROR_HANDLING() \
		php_set_error_handling(EH_THROW, opencv_ce_cvexception TSRMLS_CC)

#define PHP_OPENCV_RESTORE_ERRORS() \
		php_std_error_handling()

#endif


#ifdef ZTS
#include "TSRM.h"
#endif

#include <cv.h>
#include <highgui.h>

PHP_MINIT_FUNCTION(opencv);
PHP_MINIT_FUNCTION(opencv_error);
PHP_MINIT_FUNCTION(opencv_mat);
PHP_MINIT_FUNCTION(opencv_arr);
PHP_MINIT_FUNCTION(opencv_image);
PHP_MINIT_FUNCTION(opencv_histogram);
PHP_MINIT_FUNCTION(opencv_capture);
PHP_MSHUTDOWN_FUNCTION(opencv);
PHP_MINFO_FUNCTION(opencv);
PHP_RINIT_FUNCTION(opencv);

extern zend_object_handlers opencv_std_object_handlers;
extern zend_class_entry *opencv_ce_cvexception;
extern zend_class_entry *opencv_ce_cvmat;
extern zend_class_entry *opencv_ce_cvarr;
extern zend_class_entry *opencv_ce_image;
extern zend_class_entry *opencv_ce_histogram;

zend_error_handling opencv_original_error_handling;

typedef struct _opencv_arr_object {
	zend_object std;
	zend_bool constructed;
	CvArr *cvptr;
} opencv_arr_object;

typedef struct _opencv_mat_object {
	zend_object std;
	zend_bool constructed;
	CvMat *cvptr;
} opencv_mat_object;

typedef struct _opencv_image_object {
	zend_object std;
	zend_bool constructed;
	IplImage *cvptr;
} opencv_image_object;

typedef struct _opencv_histogram_object {
	zend_object std;
	zend_bool constructed;
	CvHistogram *cvptr;
} opencv_histogram_object;

typedef struct _opencv_capture_object {
	zend_object std;
	zend_bool constructed;
	CvCapture* cvptr;
} opencv_capture_object;


PHP_OPENCV_API extern void php_opencv_throw_exception();
PHP_OPENCV_API void php_opencv_basedir_check(const char *filename TSRMLS_DC);
PHP_OPENCV_API extern opencv_image_object* opencv_image_object_get(zval *zobj TSRMLS_DC);
PHP_OPENCV_API extern opencv_histogram_object* opencv_histogram_object_get(zval *zobj TSRMLS_DC);
PHP_OPENCV_API zval *php_opencv_make_image_zval(IplImage *image, zval *image_zval TSRMLS_DC);


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
