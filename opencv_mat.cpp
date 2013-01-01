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

zend_class_entry *opencv_ce_cvmat;

static inline opencv_mat_object* opencv_mat_object_get(zval *zobj TSRMLS_DC) {
    opencv_mat_object *pobj = (opencv_mat_object *) zend_object_store_get_object(zobj TSRMLS_CC);
    if (pobj->cvptr == NULL) {
        php_error(E_ERROR, "Internal surface object missing in %s wrapper, you must call parent::__construct in extended classes", Z_OBJCE_P(zobj)->name);
    }
    return pobj;
}

void opencv_mat_object_destroy(void *object TSRMLS_DC)
{
    opencv_mat_object *mat = (opencv_mat_object *)object;

    zend_hash_destroy(mat->std.properties);
    FREE_HASHTABLE(mat->std.properties);

    if(mat->cvptr != NULL){
        cvRelease((void **) &mat->cvptr);
    }
    efree(mat);
}

static zend_object_value opencv_mat_object_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    opencv_mat_object *mat;
    zval *temp;

    mat = (opencv_mat_object *) ecalloc(1, sizeof(opencv_mat_object));

    mat->std.ce = ce; 
    mat->cvptr = NULL;

    ALLOC_HASHTABLE(mat->std.properties);
    zend_hash_init(mat->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0); 
#if PHP_VERSION_ID < 50399
    zend_hash_copy(mat->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref,(void *) &temp, sizeof(zval *));
#else
    object_properties_init(&mat->std, ce);
#endif
    retval.handle = zend_objects_store_put(mat, NULL, (zend_objects_free_object_storage_t)opencv_mat_object_destroy, NULL TSRMLS_CC);
    retval.handlers = zend_get_std_object_handlers();
    return retval;
}

/* {{{ proto void contruct()
   OpenCV_Mat CANNOT be extended in userspace, this will throw an exception on use */
PHP_METHOD(OpenCV_Mat, __construct)
{
    long rows, cols, type;
    opencv_mat_object *object;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &rows, &cols, &type) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    object = (opencv_mat_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
    object->cvptr = cvCreateMat(rows, cols, type);
    php_opencv_throw_exception();
}
/* }}} */

/* {{{ opencv_mat_methods[] */
const zend_function_entry opencv_mat_methods[] = { 
    PHP_ME(OpenCV_Mat, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};
/* }}} */


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_mat)
{
	zend_class_entry ce;
	
    INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Mat", opencv_mat_methods);
	opencv_ce_cvarr = zend_register_internal_class_ex(&ce, opencv_ce_cvarr, NULL TSRMLS_CC);

	return SUCCESS;
}
/* }}} */


