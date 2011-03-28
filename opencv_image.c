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

PHP_OPENCV_API opencv_image_object* opencv_image_object_get(zval *zobj TSRMLS_DC) {
    opencv_image_object *pobj = zend_object_store_get_object(zobj TSRMLS_CC);
    if (pobj->cvptr == NULL) {
        php_error(E_ERROR, "Internal surface object missing in %s wrapper, you must call parent::__construct in extended classes", Z_OBJCE_P(zobj)->name);
    }
    return pobj;
}

#define PHP_OPENCV_ADD_IMAGE_LONG_PROPERTY(PROPERTY, MEMBER) \
    do { \
        zval *temp_prop; \
        MAKE_STD_ZVAL(temp_prop); \
        ZVAL_LONG(temp_prop, MEMBER); \
        zend_hash_update(Z_OBJPROP_P(image_zval), PROPERTY, sizeof(PROPERTY), (void **) &temp_prop, sizeof(zval *), NULL); \
    } while(0)


PHP_OPENCV_API zval *php_opencv_make_image_zval(IplImage *image, zval *image_zval TSRMLS_DC) {
    zval *return_value, *width, *height;
    opencv_image_object *image_obj;

    if (image_zval == NULL) {
        MAKE_STD_ZVAL(image_zval);
    }

    object_init_ex(image_zval, opencv_ce_image);
    image_obj = (opencv_image_object *) zend_object_store_get_object(image_zval TSRMLS_CC);
    image_obj->cvptr = image;

    PHP_OPENCV_ADD_IMAGE_LONG_PROPERTY("width", image->width);
    PHP_OPENCV_ADD_IMAGE_LONG_PROPERTY("height", image->height);
    PHP_OPENCV_ADD_IMAGE_LONG_PROPERTY("nChannels", image->nChannels);
    PHP_OPENCV_ADD_IMAGE_LONG_PROPERTY("alphaChannel", image->alphaChannel);
    PHP_OPENCV_ADD_IMAGE_LONG_PROPERTY("depth", image->depth);

    return image_zval;
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

/* {{{ proto void __construct(int width, int height, int format, int channels)
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

    temp = cvCreateImage(cvSize(width, height), format, channels);
    php_opencv_make_image_zval(temp, getThis() TSRMLS_CC);
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

    php_opencv_basedir_check(filename TSRMLS_CC);     

    temp = (IplImage *) cvLoadImage(filename, mode);
    if (temp == NULL) {
        char *error_message = estrdup("Could not open the video file - check it exists and the codec is available");
        zend_throw_exception(opencv_ce_cvexception, error_message, 0 TSRMLS_CC);
        efree(error_message);
        return;
    }

    php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
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
PHP_METHOD(OpenCV_Image, getImageROI) {
    opencv_image_object *image_object;
    zval *image_zval;
    CvRect rect;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &image_zval, opencv_ce_image) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(getThis() TSRMLS_CC);
    rect = cvGetImageROI(image_object->cvptr);
    array_init(return_value);
    add_assoc_long(return_value, "x", rect.x);
    add_assoc_long(return_value, "y", rect.y);
    add_assoc_long(return_value, "width", rect.width);
    add_assoc_long(return_value, "height", rect.height);

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

    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvSmooth(image_object->cvptr, dst_object->cvptr, smoothType, params[0], params[1], params[2], params[3]);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, laplace) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long apertureSize = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &apertureSize) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCreateImage(cvGetSize(image_object->cvptr), IPL_DEPTH_16S, image_object->cvptr->nChannels);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvLaplace(image_object->cvptr, dst_object->cvptr, apertureSize);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, sobel) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long xorder = 0, yorder = 0, apertureSize = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Olll", &image_zval, opencv_ce_image, &xorder, &yorder, &apertureSize) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCreateImage(cvGetSize(image_object->cvptr), IPL_DEPTH_16S, image_object->cvptr->nChannels);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvSobel(image_object->cvptr, dst_object->cvptr, xorder, yorder, apertureSize);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, erode) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvErode(image_object->cvptr, dst_object->cvptr, NULL, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, dilate) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvDilate(image_object->cvptr, dst_object->cvptr, NULL, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, open) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvMorphologyEx(image_object->cvptr, dst_object->cvptr, NULL, NULL, CV_MOP_OPEN, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, close) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvMorphologyEx(image_object->cvptr, dst_object->cvptr, NULL, NULL, CV_MOP_CLOSE, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, gradient) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp, *temp2;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    temp2 = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvMorphologyEx(image_object->cvptr, dst_object->cvptr, temp2, NULL, CV_MOP_GRADIENT, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, topHat) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvMorphologyEx(image_object->cvptr, dst_object->cvptr, NULL, NULL, CV_MOP_TOPHAT, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, blackHat) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long iterations = 1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &iterations) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCloneImage(image_object->cvptr);
    *return_value = *php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = zend_object_store_get_object(return_value TSRMLS_CC);

    cvMorphologyEx(image_object->cvptr, dst_object->cvptr, NULL, NULL, CV_MOP_BLACKHAT, iterations);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, resize) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval, *dst_zval;
    long interpolation = CV_INTER_LINEAR;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO|l", &image_zval, opencv_ce_image, &dst_zval, opencv_ce_image, &interpolation) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    dst_object = opencv_image_object_get(dst_zval TSRMLS_CC);

    cvResize(image_object->cvptr, dst_object->cvptr, interpolation);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, pyrDown) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long filter = CV_GAUSSIAN_5x5;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &filter) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCreateImage(
            cvSize(image_object->cvptr->width / 2, image_object->cvptr->height / 2),
            image_object->cvptr->depth, image_object->cvptr->nChannels);
    php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = opencv_image_object_get(return_value TSRMLS_CC);

    cvPyrDown(image_object->cvptr, dst_object->cvptr, filter);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, pyrUp) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long filter = CV_GAUSSIAN_5x5;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &image_zval, opencv_ce_image, &filter) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    temp = cvCreateImage(
            cvSize(image_object->cvptr->width * 2, image_object->cvptr->height * 2),
            image_object->cvptr->depth, image_object->cvptr->nChannels);
    php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = opencv_image_object_get(return_value TSRMLS_CC);

    cvPyrUp(image_object->cvptr, dst_object->cvptr, filter);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, canny) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp, *grey_image;
    long lowThresh, highThresh, apertureSize;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Olll", &image_zval, opencv_ce_image, &lowThresh, &highThresh, &apertureSize) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();
    
    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    if (image_object->cvptr->nChannels > 1) {
        grey_image = cvCreateImage(cvGetSize(image_object->cvptr), IPL_DEPTH_8U, 1);
        cvCvtColor(image_object->cvptr, grey_image, CV_BGR2GRAY);
    } else {
        grey_image = image_object->cvptr;
    }

    temp = cvCreateImage(cvGetSize(image_object->cvptr), IPL_DEPTH_8U, 1);
    php_opencv_make_image_zval(temp, return_value TSRMLS_CC);
    dst_object = opencv_image_object_get(return_value TSRMLS_CC);

    cvCanny(grey_image, dst_object->cvptr, lowThresh, highThresh, apertureSize);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, split) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp, *grey_image;
    long lowThresh, highThresh, apertureSize, i;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &image_zval, opencv_ce_image) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();
    
    image_object = opencv_image_object_get(image_zval TSRMLS_CC);

    IplImage **planes = calloc(image_object->cvptr->nChannels, sizeof(IplImage *));
    zval **return_zvals = calloc(image_object->cvptr->nChannels, sizeof(zval *));

    for (i = 0; i < image_object->cvptr->nChannels; i++) {
        opencv_image_object *current_plane;
        MAKE_STD_ZVAL(return_zvals[i]);
        object_init_ex(return_zvals[i], opencv_ce_image);    
        temp = cvCreateImage(cvGetSize(image_object->cvptr), IPL_DEPTH_8U, 1);
        php_opencv_make_image_zval(temp, return_zvals[i]);
        planes[i] = temp;
    }

    for (i = image_object->cvptr->nChannels; i < 4; i++) {
        planes[i] = NULL;
    }

    array_init(return_value);
    cvSplit(image_object->cvptr, planes[0], planes[1], planes[2], planes[3]);
    for (i = 0; i < image_object->cvptr->nChannels; i++) {
        add_next_index_zval(return_value, return_zvals[i]);
    }

    php_opencv_throw_exception(TSRMLS_C);
}

/* {{{ */
PHP_METHOD(OpenCV_Image, convertColor) {
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *temp;
    long code;
    long channels = -1;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &image_zval, opencv_ce_image, &code, &channels) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);

    if (channels <= 0) {
        channels = image_object->cvptr->nChannels;
    }

    temp = cvCreateImage(cvGetSize(image_object->cvptr), image_object->cvptr->depth, channels);
    cvCvtColor(image_object->cvptr, temp, code);
    php_opencv_make_image_zval(temp, return_value);

    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, backProject)
{
    opencv_image_object *image_object, *dst_object;
    opencv_histogram_object *hist_object;
    zval *image_zval, *hist_zval;
    IplImage *temp;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO", &image_zval, opencv_ce_image, &hist_zval, opencv_ce_histogram) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    hist_object = opencv_histogram_object_get(hist_zval TSRMLS_CC);

    temp = cvCloneImage(image_object->cvptr);
    cvCalcBackProject(&image_object->cvptr, temp, hist_object->cvptr);
    php_opencv_make_image_zval(temp, return_value);

    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

PHP_METHOD(OpenCV_Image, matchTemplate)
{
    opencv_image_object *image_object, *template_object, *dst_object;
    zval *image_zval, *template_zval;
    IplImage *temp;
    long mode;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OOl", &image_zval, opencv_ce_image, &template_zval, opencv_ce_image, &mode) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    template_object = opencv_image_object_get(template_zval TSRMLS_CC);

    temp = cvCreateImage(cvSize(
                image_object->cvptr->width - template_object->cvptr->width + 1,
                image_object->cvptr->height - template_object->cvptr->height + 1),
            IPL_DEPTH_32F, 1);

    cvMatchTemplate(image_object->cvptr, template_object->cvptr, temp, mode);
    php_opencv_make_image_zval(temp, return_value);
    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

PHP_METHOD(OpenCV_Image, rectangle)
{
    opencv_image_object *image_object;
    zval *image_zval, *template_zval;
    IplImage *temp;
    long x, y, width, height;
	CvScalar colour = { 255, 0, 0 };

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ollll", &image_zval, opencv_ce_image, &x, &y, &width, &height) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
	cvRectangle(image_object->cvptr, cvPoint(x, y), cvPoint(x + width, y + height), colour, 1, 8, 0);

    php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

/* {{{ */
PHP_METHOD(OpenCV_Image, haarDetectObjects)
{
    opencv_image_object *image_object, *dst_object;
    zval *image_zval;
    IplImage *grey_image;
	const char *cascade_name;
	int cascade_name_len, i;
	CvHaarClassifierCascade *cascade;
	CvMemStorage *storage = cvCreateMemStorage(0);

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &image_zval, opencv_ce_image, &cascade_name, &cascade_name_len) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

	cascade = (CvHaarClassifierCascade *) cvLoad(cascade_name, 0, 0, 0);
    
    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    if (image_object->cvptr->nChannels > 1) {
        grey_image = cvCreateImage(cvGetSize(image_object->cvptr), IPL_DEPTH_8U, 1);
        cvCvtColor(image_object->cvptr, grey_image, CV_BGR2GRAY);
    } else {
        grey_image = image_object->cvptr;
    }
	cvEqualizeHist(grey_image, grey_image);
	cvClearMemStorage(storage);
	CvSeq *objects = cvHaarDetectObjects(grey_image, cascade, storage, 1.1, 3, 0, cvSize(20, 20));

	array_init(return_value);
    for (i = 0; i < (objects ? objects->total : 0); i++) {
		zval *temp;
		CvRect *r = (CvRect *) cvGetSeqElem(objects, i);

		MAKE_STD_ZVAL(temp);
		array_init(temp);
		add_assoc_long(temp, "x", r->x);
		add_assoc_long(temp, "y", r->y);
		add_assoc_long(temp, "width", r->width);
		add_assoc_long(temp, "height", r->height);
		
        add_next_index_zval(return_value, temp);
    }

	//cvReleaseImage(grey_image);
	php_opencv_throw_exception();
}
/* }}} */

/* {{{ opencv_image_methods[] */
const zend_function_entry opencv_image_methods[] = { 
    PHP_ME(OpenCV_Image, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(OpenCV_Image, load, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(OpenCV_Image, save, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, setImageROI, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, getImageROI, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, resetImageROI, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, smooth, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, laplace, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, sobel, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, erode, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, dilate, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, open, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, gradient, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, topHat, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, blackHat, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, resize, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, pyrDown, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, pyrUp, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, canny, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, split, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, convertColor, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, backProject, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, matchTemplate, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OpenCV_Image, haarDetectObjects, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(OpenCV_Image, rectangle, NULL, ZEND_ACC_PUBLIC)
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

    REGISTER_IMAGE_LONG_CONST("INTER_NN", CV_INTER_NN);
    REGISTER_IMAGE_LONG_CONST("INTER_LINEAR", CV_INTER_LINEAR);
    REGISTER_IMAGE_LONG_CONST("INTER_AREA", CV_INTER_AREA);
    REGISTER_IMAGE_LONG_CONST("INTER_CUBIC", CV_INTER_CUBIC);

    REGISTER_IMAGE_LONG_CONST("GAUSSIAN_5x5", CV_GAUSSIAN_5x5);

    /* Constants for conversion - these should be moved into a separate class, probably */
    REGISTER_IMAGE_LONG_CONST("BGR2BGRA", CV_BGR2BGRA);
    REGISTER_IMAGE_LONG_CONST("RGB2RGBA", CV_RGB2RGBA);
    REGISTER_IMAGE_LONG_CONST("BGRA2BGR", CV_BGRA2BGR);
    REGISTER_IMAGE_LONG_CONST("RGBA2RGB", CV_RGBA2RGB);
    REGISTER_IMAGE_LONG_CONST("BGR2RGBA", CV_BGR2RGBA);
    REGISTER_IMAGE_LONG_CONST("RGB2BGRA", CV_RGB2BGRA);
    REGISTER_IMAGE_LONG_CONST("RGBA2BGR", CV_RGBA2BGR);
    REGISTER_IMAGE_LONG_CONST("BGRA2RGB", CV_BGRA2RGB);
    REGISTER_IMAGE_LONG_CONST("BGR2RGB", CV_BGR2RGB);
    REGISTER_IMAGE_LONG_CONST("RGB2BGR", CV_RGB2BGR);
    REGISTER_IMAGE_LONG_CONST("BGRA2RGBA", CV_BGRA2RGBA);
    REGISTER_IMAGE_LONG_CONST("RGBA2BGRA", CV_RGBA2BGRA);
    REGISTER_IMAGE_LONG_CONST("BGR2GRAY", CV_BGR2GRAY);
    REGISTER_IMAGE_LONG_CONST("RGB2GRAY", CV_RGB2GRAY);
    REGISTER_IMAGE_LONG_CONST("GRAY2BGR", CV_GRAY2BGR);
    REGISTER_IMAGE_LONG_CONST("GRAY2RGB", CV_GRAY2RGB);
    REGISTER_IMAGE_LONG_CONST("GRAY2BGRA", CV_GRAY2BGRA);
    REGISTER_IMAGE_LONG_CONST("GRAY2RGBA", CV_GRAY2RGBA);
    REGISTER_IMAGE_LONG_CONST("BGRA2GRAY", CV_BGRA2GRAY);
    REGISTER_IMAGE_LONG_CONST("RGBA2GRAY", CV_RGBA2GRAY);
    REGISTER_IMAGE_LONG_CONST("BGR2BGR565", CV_BGR2BGR565);
    REGISTER_IMAGE_LONG_CONST("RGB2BGR565", CV_RGB2BGR565);
    REGISTER_IMAGE_LONG_CONST("BGR5652BGR", CV_BGR5652BGR);
    REGISTER_IMAGE_LONG_CONST("BGR5652RGB", CV_BGR5652RGB);
    REGISTER_IMAGE_LONG_CONST("BGRA2BGR565", CV_BGRA2BGR565);
    REGISTER_IMAGE_LONG_CONST("RGBA2BGR565", CV_RGBA2BGR565);
    REGISTER_IMAGE_LONG_CONST("BGR5652BGRA", CV_BGR5652BGRA);
    REGISTER_IMAGE_LONG_CONST("BGR5652RGBA", CV_BGR5652RGBA);
    REGISTER_IMAGE_LONG_CONST("GRAY2BGR565", CV_GRAY2BGR565);
    REGISTER_IMAGE_LONG_CONST("BGR5652GRAY", CV_BGR5652GRAY);
    REGISTER_IMAGE_LONG_CONST("BGR2BGR555", CV_BGR2BGR555);
    REGISTER_IMAGE_LONG_CONST("RGB2BGR555", CV_RGB2BGR555);
    REGISTER_IMAGE_LONG_CONST("BGR5552BGR", CV_BGR5552BGR);
    REGISTER_IMAGE_LONG_CONST("BGR5552RGB", CV_BGR5552RGB);
    REGISTER_IMAGE_LONG_CONST("BGRA2BGR555", CV_BGRA2BGR555);
    REGISTER_IMAGE_LONG_CONST("RGBA2BGR555", CV_RGBA2BGR555);
    REGISTER_IMAGE_LONG_CONST("BGR5552BGRA", CV_BGR5552BGRA);
    REGISTER_IMAGE_LONG_CONST("BGR5552RGBA", CV_BGR5552RGBA);
    REGISTER_IMAGE_LONG_CONST("GRAY2BGR555", CV_GRAY2BGR555);
    REGISTER_IMAGE_LONG_CONST("BGR5552GRAY", CV_BGR5552GRAY);
    REGISTER_IMAGE_LONG_CONST("BGR2XYZ", CV_BGR2XYZ);
    REGISTER_IMAGE_LONG_CONST("RGB2XYZ", CV_RGB2XYZ);
    REGISTER_IMAGE_LONG_CONST("XYZ2BGR", CV_XYZ2BGR);
    REGISTER_IMAGE_LONG_CONST("XYZ2RGB", CV_XYZ2RGB);
    REGISTER_IMAGE_LONG_CONST("BGR2YCrCb", CV_BGR2YCrCb);
    REGISTER_IMAGE_LONG_CONST("RGB2YCrCb", CV_RGB2YCrCb);
    REGISTER_IMAGE_LONG_CONST("YCrCb2BGR", CV_YCrCb2BGR);
    REGISTER_IMAGE_LONG_CONST("YCrCb2RGB", CV_YCrCb2RGB);
    REGISTER_IMAGE_LONG_CONST("BGR2HSV", CV_BGR2HSV);
    REGISTER_IMAGE_LONG_CONST("RGB2HSV", CV_RGB2HSV);
    REGISTER_IMAGE_LONG_CONST("BGR2Lab", CV_BGR2Lab);
    REGISTER_IMAGE_LONG_CONST("RGB2Lab", CV_RGB2Lab);
    REGISTER_IMAGE_LONG_CONST("BayerBG2BGR", CV_BayerBG2BGR);
    REGISTER_IMAGE_LONG_CONST("BayerGB2BGR", CV_BayerGB2BGR);
    REGISTER_IMAGE_LONG_CONST("BayerRG2BGR", CV_BayerRG2BGR);
    REGISTER_IMAGE_LONG_CONST("BayerGR2BGR", CV_BayerGR2BGR);
    REGISTER_IMAGE_LONG_CONST("BayerBG2RGB", CV_BayerBG2RGB);
    REGISTER_IMAGE_LONG_CONST("BayerGB2RGB", CV_BayerGB2RGB);
    REGISTER_IMAGE_LONG_CONST("BayerRG2RGB", CV_BayerRG2RGB);
    REGISTER_IMAGE_LONG_CONST("BayerGR2RGB", CV_BayerGR2RGB);
    REGISTER_IMAGE_LONG_CONST("BGR2Luv", CV_BGR2Luv);
    REGISTER_IMAGE_LONG_CONST("RGB2Luv", CV_RGB2Luv);
    REGISTER_IMAGE_LONG_CONST("BGR2HLS", CV_BGR2HLS);
    REGISTER_IMAGE_LONG_CONST("RGB2HLS", CV_RGB2HLS);
    REGISTER_IMAGE_LONG_CONST("HSV2BGR", CV_HSV2BGR);
    REGISTER_IMAGE_LONG_CONST("HSV2RGB", CV_HSV2RGB);
    REGISTER_IMAGE_LONG_CONST("Lab2BGR", CV_Lab2BGR);
    REGISTER_IMAGE_LONG_CONST("Lab2RGB", CV_Lab2RGB);
    REGISTER_IMAGE_LONG_CONST("Luv2BGR", CV_Luv2BGR);
    REGISTER_IMAGE_LONG_CONST("Luv2RGB", CV_Luv2RGB);
    REGISTER_IMAGE_LONG_CONST("HLS2BGR", CV_HLS2BGR);
    REGISTER_IMAGE_LONG_CONST("HLS2RGB", CV_HLS2RGB);

    REGISTER_IMAGE_LONG_CONST("TM_SQDIFF", CV_TM_SQDIFF);
    REGISTER_IMAGE_LONG_CONST("TM_SQDIFF_NORMED", CV_TM_SQDIFF_NORMED);
    REGISTER_IMAGE_LONG_CONST("TM_CCORR", CV_TM_CCORR);
    REGISTER_IMAGE_LONG_CONST("TM_CCORR_NORMED", CV_TM_CCORR_NORMED);
    REGISTER_IMAGE_LONG_CONST("TM_CCOEFF", CV_TM_CCOEFF);
    REGISTER_IMAGE_LONG_CONST("TM_CCOEFF_NORMED", CV_TM_CCOEFF_NORMED);

	return SUCCESS;
}
/* }}} */


