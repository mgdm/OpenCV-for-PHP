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

zend_class_entry *opencv_ce_capture;

PHP_OPENCV_API opencv_capture_object* opencv_capture_object_get(zval *zobj TSRMLS_DC) {
    opencv_capture_object *pobj = (opencv_capture_object *) zend_object_store_get_object(zobj TSRMLS_CC);
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

    capture = (opencv_capture_object *) ecalloc(1, sizeof(opencv_capture_object));

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

PHP_METHOD(OpenCV_Capture, createCameraCapture)
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

    object_init_ex(return_value, opencv_ce_capture);
    temp = (CvCapture *) cvCaptureFromCAM(camera);
    capture_object = (opencv_capture_object *) zend_object_store_get_object(return_value TSRMLS_CC);
    capture_object->cvptr = temp;

	php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

PHP_METHOD(OpenCV_Capture, createFileCapture)
{
    const char *filename;
	int filename_len;
	opencv_capture_object *capture_object;
    CvCapture *temp;

	PHP_OPENCV_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE)
    {
		PHP_OPENCV_RESTORE_ERRORS();
		return;
	}
	PHP_OPENCV_RESTORE_ERRORS();

    php_opencv_basedir_check(filename TSRMLS_CC);

    object_init_ex(return_value, opencv_ce_capture);
    capture_object = (opencv_capture_object *) zend_object_store_get_object(return_value TSRMLS_CC);
    temp = (CvCapture *) cvCreateFileCapture(filename);

    if (temp == NULL) {
        char *error_message = estrdup("Could not open the video file - check it exists and the codec is available");
        zend_throw_exception(opencv_ce_cvexception, error_message, 0 TSRMLS_CC);
        efree(error_message);
        return;
    }

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

PHP_METHOD(OpenCV_Capture, getProperty)
{
    zval *capture_zval;
    opencv_capture_object *capture_object;
    IplImage *temp;
    double val;
    long property;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &capture_zval, opencv_ce_capture, &property) == FAILURE)
    {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_ERROR_HANDLING();

    capture_object = opencv_capture_object_get(getThis() TSRMLS_CC);
    val = cvGetCaptureProperty(capture_object->cvptr, property);
    php_opencv_throw_exception();

    /* FourCC is special */
    if (property == CV_CAP_PROP_FOURCC) {
        char *unpacked_val = (char *) &val;
        RETURN_STRING(unpacked_val, 1);
    }
    RETURN_LONG(val);
}

PHP_METHOD(OpenCV_Capture, setProperty)
{
    zval *capture_zval;
    opencv_capture_object *capture_object;
    IplImage *temp;
    double val;
    long property;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Old", &capture_zval, opencv_ce_capture, &property, &val) == FAILURE)
    {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_ERROR_HANDLING();

    capture_object = opencv_capture_object_get(getThis() TSRMLS_CC);
    val = cvSetCaptureProperty(capture_object->cvptr, property, val);
    php_opencv_throw_exception();
}

/* {{{ opencv_capture_methods[] */
const zend_function_entry opencv_capture_methods[] = {
    PHP_ME(OpenCV_Capture, createCameraCapture, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(OpenCV_Capture, createFileCapture, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(OpenCV_Capture, grabFrame, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Capture, retrieveFrame, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Capture, queryFrame, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Capture, getProperty, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Capture, setProperty, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_capture)
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Capture", opencv_capture_methods);
	opencv_ce_capture = zend_register_internal_class(&ce TSRMLS_CC);

    #define REGISTER_CAPTURE_LONG_CONST(const_name, value) \
	zend_declare_class_constant_long(opencv_ce_capture, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC); \
	REGISTER_LONG_CONSTANT(#value,  value,  CONST_CS | CONST_PERSISTENT);

    REGISTER_CAPTURE_LONG_CONST("PROP_POS_MSEC", CV_CAP_PROP_POS_MSEC);
    REGISTER_CAPTURE_LONG_CONST("PROP_POS_FRAMES", CV_CAP_PROP_POS_FRAMES);
    REGISTER_CAPTURE_LONG_CONST("PROP_POS_AVI_RATIO", CV_CAP_PROP_POS_AVI_RATIO);
    REGISTER_CAPTURE_LONG_CONST("PROP_FRAME_WIDTH", CV_CAP_PROP_FRAME_WIDTH);
    REGISTER_CAPTURE_LONG_CONST("PROP_FRAME_HEIGHT", CV_CAP_PROP_FRAME_HEIGHT);
    REGISTER_CAPTURE_LONG_CONST("PROP_FPS", CV_CAP_PROP_FPS);
    REGISTER_CAPTURE_LONG_CONST("PROP_FOURCC", CV_CAP_PROP_FOURCC);
    REGISTER_CAPTURE_LONG_CONST("PROP_FRAME_COUNT", CV_CAP_PROP_FRAME_COUNT);
    REGISTER_CAPTURE_LONG_CONST("PROP_FORMAT", CV_CAP_PROP_FORMAT);
    REGISTER_CAPTURE_LONG_CONST("PROP_MODE", CV_CAP_PROP_MODE);
    REGISTER_CAPTURE_LONG_CONST("PROP_BRIGHTNESS", CV_CAP_PROP_BRIGHTNESS);
    REGISTER_CAPTURE_LONG_CONST("PROP_CONTRAST", CV_CAP_PROP_CONTRAST);
    REGISTER_CAPTURE_LONG_CONST("PROP_SATURATION", CV_CAP_PROP_SATURATION);
    REGISTER_CAPTURE_LONG_CONST("PROP_HUE", CV_CAP_PROP_HUE);
    REGISTER_CAPTURE_LONG_CONST("PROP_GAIN", CV_CAP_PROP_GAIN);
    REGISTER_CAPTURE_LONG_CONST("PROP_EXPOSURE", CV_CAP_PROP_EXPOSURE);
    REGISTER_CAPTURE_LONG_CONST("PROP_CONVERT_RGB", CV_CAP_PROP_CONVERT_RGB);

    #ifdef CV_CAP_PROP_WHITE_BALANCE
    REGISTER_CAPTURE_LONG_CONST("PROP_WHITE_BALANCE", CV_CAP_PROP_WHITE_BALANCE);
    #endif

    #ifdef CV_CAP_PROP_WHITE_BALANCE_BLUE_U
    REGISTER_CAPTURE_LONG_CONST("PROP_WHITE_BALANCE_BLUE_U", CV_CAP_PROP_WHITE_BALANCE_BLUE_U);
    #endif

    REGISTER_CAPTURE_LONG_CONST("PROP_RECTIFICATION", CV_CAP_PROP_RECTIFICATION);

    #ifdef CV_CAP_PROP_MONOCROME
    REGISTER_CAPTURE_LONG_CONST("PROP_MONOCHROME", CV_CAP_PROP_MONOCROME);
    #endif

    #ifdef CV_CAP_PROP_SHARPNESS
    REGISTER_CAPTURE_LONG_CONST("PROP_SHARPNESS", CV_CAP_PROP_SHARPNESS);
    #endif

    #ifdef CV_CAP_PROP_AUTO_EXPOSURE
    REGISTER_CAPTURE_LONG_CONST("PROP_AUTO_EXPOSURE", CV_CAP_PROP_AUTO_EXPOSURE);
    #endif

    #ifdef CV_CAP_PROP_GAMMA
    REGISTER_CAPTURE_LONG_CONST("PROP_GAMMA", CV_CAP_PROP_GAMMA);
    #endif

    #ifdef CV_CAP_PROP_TEMPERATURE
    REGISTER_CAPTURE_LONG_CONST("PROP_TEMPERATURE", CV_CAP_PROP_TEMPERATURE);
    #endif

    #ifdef CV_CAP_PROP_GAMMA
    REGISTER_CAPTURE_LONG_CONST("PROP_GAMMA", CV_CAP_PROP_GAMMA);
    #endif

    #ifdef CV_CAP_PROP_TRIGGER
    REGISTER_CAPTURE_LONG_CONST("PROP_TRIGGER", CV_CAP_PROP_TRIGGER);
    #endif

    #ifdef CV_CAP_PROP_TRIGGER_DELAY
    REGISTER_CAPTURE_LONG_CONST("PROP_TRIGGER_DELAY", CV_CAP_PROP_TRIGGER_DELAY);
    #endif

    #ifdef CV_CAP_PROP_WHITE_BALANCE_RED_V
    REGISTER_CAPTURE_LONG_CONST("PROP_WHITE_BALANCE_RED_V", CV_CAP_PROP_WHITE_BALANCE_RED_V);
    #endif

    #ifdef CV_CAP_PROP_MAX_DC1394
    REGISTER_CAPTURE_LONG_CONST("PROP_MAX_DC1394", CV_CAP_PROP_MAX_DC1394);
    #endif

	return SUCCESS;
}
/* }}} */
