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

extern "C" {
#include "php.h"
#include "zend_exceptions.h"

#ifdef ZTS
#include "TSRM.h"
#endif

}

extern zend_error_handling opencv_original_error_handling;
extern zend_module_entry opencv_module_entry;
#define phpext_opencv_ptr &opencv_module_entry


#ifdef PHP_WIN32
#	define PHP_OPENCV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_OPENCV_API __attribute__ ((visibility("default")))
#else
#	define PHP_OPENCV_API
#endif

/* Constants for open_basedir checks */
#define OPENCV_READ_WRITE_NO_ERROR 0
#define OPENCV_READ_WRITE_SAFE_MODE_ERROR 1
#define OPENCV_READ_WRITE_OPEN_BASEDIR_ERROR 2

/* turn error handling to exception mode and restore */
#define PHP_OPENCV_ERROR_HANDLING() do { \
	zend_replace_error_handling(EH_THROW, opencv_ce_cvexception, &opencv_original_error_handling TSRMLS_CC); \
} while(0)

#define PHP_OPENCV_RESTORE_ERRORS() do { \
	zend_restore_error_handling(&opencv_original_error_handling TSRMLS_CC); \
} while(0)

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

PHP_MINIT_FUNCTION(opencv);
PHP_MINIT_FUNCTION(opencv_error);
PHP_MINIT_FUNCTION(opencv_mat);
PHP_MINIT_FUNCTION(opencv_image);
PHP_MINIT_FUNCTION(opencv_histogram);
PHP_MINIT_FUNCTION(opencv_capture);
PHP_MSHUTDOWN_FUNCTION(opencv);
PHP_MINFO_FUNCTION(opencv);
PHP_RINIT_FUNCTION(opencv);

extern zend_object_handlers opencv_std_object_handlers;
extern zend_class_entry *opencv_ce_cvexception;
extern zend_class_entry *opencv_ce_cvmat;
extern zend_class_entry *opencv_ce_image;
extern zend_class_entry *opencv_ce_histogram;


typedef struct _opencv_mat_object {
	zend_object std;
	zend_bool constructed;
	Mat *cvptr;
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
