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
#include <highgui.h>

zend_class_entry *opencv_ce_capture;

PHP_OPENCV_API opencv_capture_object* opencv_capture_object_get(zval *zobj TSRMLS_DC) {
    opencv_capture_object *pobj = zend_object_store_get_object(zobj TSRMLS_CC);
    if (pobj->cvptr == NULL) {
        php_error(E_ERROR, "Internal surface object missing in %s wrapper, you must call parent::__construct in extended classes", Z_OBJCE_P(zobj)->name);
    }
    return pobj;
}

void opencv_capture_object_destroy(void *object TSRMLS_DC)
{
    opencv_capture_object *capture = (opencv_capture_object *)object;

    zend_hash_destroy(capture->std.properties);
    FREE_HASHTABLE(capture->std.properties);

    if(capture->cvptr != NULL){
        cvReleaseCapture(&capture->cvptr);
    }
    efree(capture);
}

PHP_OPENCV_API zend_object_value opencv_capture_object_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    opencv_capture_object *capture;
    zval *temp;

    capture = ecalloc(1, sizeof(opencv_capture_object));

    capture->std.ce = ce; 
    capture->cvptr = NULL;

    ALLOC_HASHTABLE(capture->std.properties);
    zend_hash_init(capture->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0); 
#if PHP_VERSION_ID < 50399
    zend_hash_copy(capture->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &temp, sizeof(zval *));
#else
    object_properties_init(&capture->std, ce);
#endif
    retval.handle = zend_objects_store_put(capture, NULL, (zend_objects_free_object_storage_t)opencv_capture_object_destroy, NULL TSRMLS_CC);
    retval.handlers = zend_get_std_object_handlers();
    return retval;
}

PHP_METHOD(OpenCV_Capture, __construct)
{
	long camera;
	opencv_capture_object *capture_object;
    CvCapture *temp;

	PHP_OPENCV_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &camera) == FAILURE)
    {
		PHP_OPENCV_RESTORE_ERRORS();
		return;
	}
	PHP_OPENCV_RESTORE_ERRORS();

    temp = (CvCapture *) cvCaptureFromCAM(camera);
    capture_object = zend_object_store_get_object(getThis() TSRMLS_CC);
    capture_object->cvptr = temp;
    
	php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

PHP_METHOD(OpenCV_Capture, grabFrame)
{
    zval *capture_zval;
    opencv_capture_object *capture_object;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &capture_zval, opencv_ce_capture) == FAILURE)
    {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_ERROR_HANDLING();

    capture_object = opencv_capture_object_get(getThis() TSRMLS_CC);
    long result = cvGrabFrame(capture_object->cvptr);

    php_opencv_throw_exception();
    RETURN_LONG(result);
}

PHP_METHOD(OpenCV_Capture, retrieveFrame)
{
    zval *capture_zval;
    opencv_capture_object *capture_object;
    IplImage *temp;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &capture_zval, opencv_ce_capture) == FAILURE)
    {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_ERROR_HANDLING();

    capture_object = opencv_capture_object_get(getThis() TSRMLS_CC);
    temp = cvCloneImage(cvRetrieveFrame(capture_object->cvptr, 0));
    php_opencv_make_image_zval(temp, return_value);

    php_opencv_throw_exception();
}

PHP_METHOD(OpenCV_Capture, queryFrame)
{
    zval *capture_zval;
    opencv_capture_object *capture_object;
    IplImage *temp;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &capture_zval, opencv_ce_capture) == FAILURE)
    {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_ERROR_HANDLING();

    capture_object = opencv_capture_object_get(getThis() TSRMLS_CC);
    temp = cvCloneImage(cvQueryFrame(capture_object->cvptr));
    php_opencv_make_image_zval(temp, return_value);

    php_opencv_throw_exception();
}

/* {{{ opencv_capture_methods[] */
const zend_function_entry opencv_capture_methods[] = { 
    PHP_ME(OpenCV_Capture, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(OpenCV_Capture, grabFrame, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Capture, retrieveFrame, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Capture, queryFrame, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_capture)
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Capture", opencv_capture_methods);
	opencv_ce_capture = zend_register_internal_class(&ce TSRMLS_CC);
	
    #define REGISTER_HIST_LONG_CONST(const_name, value) \
	zend_declare_class_constant_long(opencv_ce_capture, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC); \
	REGISTER_LONG_CONSTANT(#value,  value,  CONST_CS | CONST_PERSISTENT);

	REGISTER_HIST_LONG_CONST("TYPE_SPARSE", CV_HIST_SPARSE);
    REGISTER_HIST_LONG_CONST("TYPE_ARRAY", CV_HIST_ARRAY);

	return SUCCESS;
}
/* }}} */
