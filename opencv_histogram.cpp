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

zend_class_entry *opencv_ce_histogram;

PHP_OPENCV_API opencv_histogram_object* opencv_histogram_object_get(zval *zobj TSRMLS_DC) {
    opencv_histogram_object *pobj = (opencv_histogram_object *) zend_object_store_get_object(zobj TSRMLS_CC);
    if (pobj->cvptr == NULL) {
        php_error(E_ERROR, "Internal surface object missing in %s wrapper, you must call parent::__construct in extended classes", Z_OBJCE_P(zobj)->name);
    }
    return pobj;
}

void opencv_histogram_object_destroy(void *object TSRMLS_DC)
{
    opencv_histogram_object *histogram = (opencv_histogram_object *)object;

    zend_hash_destroy(histogram->std.properties);
    FREE_HASHTABLE(histogram->std.properties);

    if(histogram->cvptr != NULL){
        cvReleaseHist(&histogram->cvptr);
    }
    efree(histogram);
}

PHP_OPENCV_API zend_object_value opencv_histogram_object_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    opencv_histogram_object *histogram;
    zval *temp;

    histogram = (opencv_histogram_object *) ecalloc(1, sizeof(opencv_histogram_object));

    histogram->std.ce = ce; 
    histogram->cvptr = NULL;

    ALLOC_HASHTABLE(histogram->std.properties);
    zend_hash_init(histogram->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0); 
#if PHP_VERSION_ID < 50399
    zend_hash_copy(histogram->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &temp, sizeof(zval *));
#else
    object_properties_init(&histogram->std, ce);
#endif
    retval.handle = zend_objects_store_put(histogram, NULL, (zend_objects_free_object_storage_t)opencv_histogram_object_destroy, NULL TSRMLS_CC);
    retval.handlers = zend_get_std_object_handlers();
    return retval;
}

PHP_METHOD(OpenCV_Histogram, __construct)
{
	long bins, sizes, type, uniform;
    zval *ranges;
	opencv_histogram_object *histogram_object;
    CvHistogram *temp;
    int cast_sizes;

	PHP_OPENCV_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll|al", &bins, &sizes, &type, &ranges, &uniform) == FAILURE)
    {
		PHP_OPENCV_RESTORE_ERRORS();
		return;
	}
	PHP_OPENCV_RESTORE_ERRORS();

    cast_sizes = sizes;

    temp = cvCreateHist(bins, &cast_sizes, type, NULL, 1);
    histogram_object = (opencv_histogram_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
    histogram_object->cvptr = temp;
    
	php_opencv_throw_exception(TSRMLS_C);
}
/* }}} */

PHP_METHOD(OpenCV_Histogram, calc)
{
    zval *hist_zval, *image_zval;
    opencv_histogram_object *hist_object;
    opencv_image_object *image_object;
    long accumulate = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OO|l", &hist_zval, opencv_ce_histogram, &image_zval, opencv_ce_image) == FAILURE)
    {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_ERROR_HANDLING();

    hist_object = opencv_histogram_object_get(getThis() TSRMLS_CC);
    image_object = opencv_image_object_get(image_zval TSRMLS_CC);
    cvCalcHist(&image_object->cvptr, hist_object->cvptr, accumulate, NULL);

    php_opencv_throw_exception(TSRMLS_C);
}

/* {{{ opencv_histogram_methods[] */
const zend_function_entry opencv_histogram_methods[] = { 
    PHP_ME(OpenCV_Histogram, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(OpenCV_Histogram, calc, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_histogram)
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Histogram", opencv_histogram_methods);
	opencv_ce_histogram = zend_register_internal_class(&ce TSRMLS_CC);
	
    #define REGISTER_HIST_LONG_CONST(const_name, value) \
	zend_declare_class_constant_long(opencv_ce_histogram, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC); \
	REGISTER_LONG_CONSTANT(#value,  value,  CONST_CS | CONST_PERSISTENT);

	REGISTER_HIST_LONG_CONST("TYPE_SPARSE", CV_HIST_SPARSE);
    REGISTER_HIST_LONG_CONST("TYPE_ARRAY", CV_HIST_ARRAY);

	return SUCCESS;
}
/* }}} */
