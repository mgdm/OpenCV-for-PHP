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

zend_class_entry *opencv_ce_image;

static inline opencv_image_object* opencv_image_object_get(zval *zobj TSRMLS_DC) {
    opencv_image_object *pobj = zend_object_store_get_object(zobj TSRMLS_CC);
    if (pobj->cvptr == NULL) {
        php_error(E_ERROR, "Internal surface object missing in %s wrapper, you must call parent::__construct in extended classes", Z_OBJCE_P(zobj)->name);
    }
    return pobj;
}

#define PHP_OPENCV_ADD_IMAGE_PROPERTY(PROPERTY, MEMBER) \
    do { \
        zval *temp_prop; \
        MAKE_STD_ZVAL(temp_prop); \
        ZVAL_LONG(temp_prop, MEMBER); \
        zend_hash_update(Z_OBJPROP_P(return_value), PROPERTY, sizeof(PROPERTY), (void **) &temp_prop, sizeof(zval *), NULL); \
    } while(0);


PHP_OPENCV_API zval *php_opencv_make_image_zval(IplImage *image TSRMLS_DC) {
    zval *return_value, *width, *height;
    opencv_image_object *image_obj;

    MAKE_STD_ZVAL(return_value);
    object_init_ex(return_value, opencv_ce_image);
    image_obj = (opencv_image_object *) zend_object_store_get_object(return_value TSRMLS_CC);
    image_obj->cvptr = image;

    PHP_OPENCV_ADD_IMAGE_PROPERTY("width", image->width);
    PHP_OPENCV_ADD_IMAGE_PROPERTY("height", image->height);
    PHP_OPENCV_ADD_IMAGE_PROPERTY("nChannels", image->nChannels);
    PHP_OPENCV_ADD_IMAGE_PROPERTY("alphaChannel", image->alphaChannel);
    PHP_OPENCV_ADD_IMAGE_PROPERTY("depth", image->depth);

    return return_value;
}

void opencv_image_object_destroy(void *object TSRMLS_DC)
{
    opencv_image_object *image = (opencv_image_object *)object;

    zend_hash_destroy(image->std.properties);
    FREE_HASHTABLE(image->std.properties);

    if(image->cvptr != NULL){
        cvReleaseImage(&image->cvptr);
    }
    efree(image);
}

static zend_object_value opencv_image_object_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    opencv_image_object *image;
    zval *temp;

    image = ecalloc(1, sizeof(opencv_image_object));

    image->std.ce = ce; 
    image->cvptr = NULL;

    ALLOC_HASHTABLE(image->std.properties);
    zend_hash_init(image->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0); 
#if PHP_VERSION_ID < 50399
    zend_hash_copy(image->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &temp, sizeof(zval *));
#else
    object_properties_init(&image->std, ce);
#endif
    retval.handle = zend_objects_store_put(image, NULL, (zend_objects_free_object_storage_t)opencv_image_object_destroy, NULL TSRMLS_CC);
    retval.handlers = zend_get_std_object_handlers();
    return retval;
}

/* {{{ proto void __construct(int format, int width, int height)
       Returns new CairoSurfaceImage object created on an image surface */
PHP_METHOD(OpenCV_Image, __construct)
{
	long format, width, height, channels;
	opencv_image_object *image_object;
    IplImage *temp;

	PHP_OPENCV_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &width, &height, &format, &channels) == FAILURE)
    {
		PHP_OPENCV_RESTORE_ERRORS();
		return;
	}
	PHP_OPENCV_RESTORE_ERRORS();

    zval_dtor(return_value);
    temp = cvCreateImage(cvSize(width, height), format, channels);
    *return_value = *php_opencv_make_image_zval(temp);
	image_object = (opencv_image_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

PHP_METHOD(OpenCV_Image, load) {
    IplImage *temp;
    char *filename;
    int filename_len;
    long mode = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &filename, &filename_len, &mode) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    zval_dtor(return_value);
    temp = (IplImage *) cvLoadImage(filename, mode);
    *return_value = *php_opencv_make_image_zval(temp);
    php_opencv_throw_exception(TSRMLS_C);
}

PHP_METHOD(OpenCV_Image, save) {
    opencv_image_object *image_object;
    zval *image_zval = NULL;
    char *filename;
    int filename_len, status, cast_mode;
    long mode = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|l", &image_zval, opencv_ce_image, &filename, &filename_len, &mode) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(getThis() TSRMLS_CC);
    cast_mode = mode;
    cvSetErrMode(CV_ErrModeSilent);
    status = cvSaveImage(filename, image_object->cvptr, 0);
    php_opencv_throw_exception(TSRMLS_C);

    if (status == 0) {
        zend_throw_exception(opencv_ce_cvexception, "Failed to save image", 0 TSRMLS_CC);
    }
}

/* {{{ */
PHP_METHOD(OpenCV_Image, setImageROI) {
    opencv_image_object *image_object;
    zval *image_zval, *rect_zval, **ppzval;
    HashTable *rect_ht;
    long rect_vals[4] = {0, 0, 0, 0};
    int i = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ollll", &image_zval, opencv_ce_image, &rect_vals[0], &rect_vals[1], &rect_vals[2], &rect_vals[3]) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(getThis() TSRMLS_CC);
    cvSetImageROI(image_object->cvptr, cvRect(rect_vals[0], rect_vals[1], rect_vals[2], rect_vals[3]));
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */



/* {{{ */
PHP_METHOD(OpenCV_Image, resetImageROI) {
    opencv_image_object *image_object;
    zval *image_zval;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(getThis() TSRMLS_CC);
    cvResetImageROI(image_object->cvptr);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, smooth) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long params[4] = { 3, 0, 0, 0 };
    long smoothType = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Olllll", &image_zval, opencv_ce_image, &smoothType, &params[0], &params[1], &params[2], &params[3]) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);

    zval_dtor (return_value);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvSmooth(image_object->cvptr, dst_object->cvptr, smoothType, params[0], params[1], params[2], params[3]);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ opencv_image_methods[] */
const zend_function_entry opencv_image_methods[] = { 
    //PHP_ME(OpenCV_Image, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_STATIC)
    PHP_ME(OpenCV_Image, load, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(OpenCV_Image, save, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, setImageROI, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, resetImageROI, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, smooth, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_image)
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Image", opencv_image_methods);
	opencv_ce_image = zend_register_internal_class_ex(&ce, opencv_ce_cvarr, NULL TSRMLS_CC);
    opencv_ce_image->create_object = opencv_image_object_new;

	#define REGISTER_IMAGE_LONG_CONST(const_name, value) \
	zend_declare_class_constant_long(opencv_ce_image, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC); \
	REGISTER_LONG_CONSTANT(#value,  value,  CONST_CS | CONST_PERSISTENT);

	REGISTER_IMAGE_LONG_CONST("DEPTH_8U", IPL_DEPTH_8U);
	REGISTER_IMAGE_LONG_CONST("DEPTH_8S", IPL_DEPTH_8S);
	REGISTER_IMAGE_LONG_CONST("DEPTH_16U", IPL_DEPTH_16U);
	REGISTER_IMAGE_LONG_CONST("DEPTH_16S", IPL_DEPTH_16S);
	REGISTER_IMAGE_LONG_CONST("DEPTH_32S", IPL_DEPTH_32S);
	REGISTER_IMAGE_LONG_CONST("DEPTH_32F", IPL_DEPTH_32F);
	REGISTER_IMAGE_LONG_CONST("DEPTH_64F", IPL_DEPTH_64F);

    REGISTER_IMAGE_LONG_CONST("LOAD_IMAGE_COLOR", CV_LOAD_IMAGE_COLOR);
    REGISTER_IMAGE_LONG_CONST("LOAD_IMAGE_GRAYSCALE", CV_LOAD_IMAGE_GRAYSCALE);
    REGISTER_IMAGE_LONG_CONST("LOAD_IMAGE_UNCHANGED", CV_LOAD_IMAGE_UNCHANGED);

    REGISTER_IMAGE_LONG_CONST("BLUR_NO_SCALE", CV_BLUR_NO_SCALE);
    REGISTER_IMAGE_LONG_CONST("BLUR", CV_BLUR);
    REGISTER_IMAGE_LONG_CONST("GAUSSIAN", CV_GAUSSIAN);
    REGISTER_IMAGE_LONG_CONST("MEDIAN", CV_MEDIAN);
    REGISTER_IMAGE_LONG_CONST("BILATERAL", CV_BILATERAL);

	return SUCCESS;
}
/* }}} */


