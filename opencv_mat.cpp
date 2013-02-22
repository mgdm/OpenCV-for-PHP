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
    if (pobj->cvptr->empty()) {
        php_error(E_ERROR, "Internal surface object missing in %s wrapper, you must call parent::__construct in extended classes", Z_OBJCE_P(zobj)->name);
    }
    return pobj;
}

#define PHP_OPENCV_ADD_MAT_LONG_PROPERTY(PROPERTY, MEMBER) \
    do { \
        zval *temp_prop; \
        MAKE_STD_ZVAL(temp_prop); \
        ZVAL_LONG(temp_prop, MEMBER); \
        zend_hash_update(Z_OBJPROP_P(mat_zval), PROPERTY, sizeof(PROPERTY), (void **) &temp_prop, sizeof(zval *), NULL); \
    } while(0)

/*
PHP_OPENCV_API zval *php_opencv_make_mat_zval(Mat mat, zval *mat_zval TSRMLS_DC) {
    zval *return_value, *width, *height;
    opencv_mat_object *mat_obj;

    if (mat_zval == NULL) {
        MAKE_STD_ZVAL(mat_zval);
    }

    object_init_ex(mat_zval, opencv_ce_cvmat);
    mat_obj = (opencv_mat_object *) zend_object_store_get_object(mat_zval TSRMLS_CC);
    mat_obj->cvptr = &mat;

    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("cols", mat.cols);
    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("rows", mat.rows);
    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("channels", mat.channels());
//    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("alphaChannel", mat.alphaChannel);
    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("depth", mat.depth());

    return mat_zval;
}
*/

static void opencv_mat_object_assign_properties(zval *mat_zval TSRMLS_DC) {
	opencv_mat_object *mat_obj;
    mat_obj = (opencv_mat_object *) zend_object_store_get_object(mat_zval TSRMLS_CC);

    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("cols", mat_obj->cvptr->cols);
    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("rows", mat_obj->cvptr->rows);
    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("channels", mat_obj->cvptr->channels());
//    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("alphaChannel", mat_obj->cvptr.alphaChannel);
    PHP_OPENCV_ADD_MAT_LONG_PROPERTY("depth", mat_obj->cvptr->depth());
}

void opencv_mat_object_destroy(void *object TSRMLS_DC)
{
    opencv_mat_object *mat = (opencv_mat_object *)object;
	//delete mat->cvptr;
    zend_hash_destroy(mat->std.properties);
    FREE_HASHTABLE(mat->std.properties);

	/*
    if(mat->cvptr != NULL){
        cvRelease((void **) &mat->cvptr);
    }
	*/
    efree(mat);
}

static zend_object_value opencv_mat_object_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    opencv_mat_object *mat;
    zval *temp;

    mat = (opencv_mat_object *) ecalloc(1, sizeof(opencv_mat_object));

    mat->std.ce = ce; 
    //mat->cvptr = NULL;

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
    object->cvptr = new Mat(rows, cols, type);
	opencv_mat_object_assign_properties(getThis() TSRMLS_CC);
    php_opencv_throw_exception();
}
/* }}} */

PHP_METHOD(OpenCV_Mat, load) {
	Mat temp;
    char *filename;
    int filename_len;
    long mode = 0;
	opencv_mat_object *mat_obj;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &filename, &filename_len, &mode) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    php_opencv_basedir_check(filename TSRMLS_CC);

    object_init_ex(return_value, opencv_ce_cvmat);
    mat_obj = (opencv_mat_object *) zend_object_store_get_object(return_value TSRMLS_CC);

    temp = imread(filename, mode);
    if (temp.empty()) {
        char *error_message = estrdup("Could not open the video file - check it exists and the codec is available");
        zend_throw_exception(opencv_ce_cvexception, error_message, 0 TSRMLS_CC);
        efree(error_message);
        return;
    }

	// I'm sure there's a neater way to do this
	mat_obj->cvptr = new Mat(temp);
	opencv_mat_object_assign_properties(return_value TSRMLS_CC);

    php_opencv_throw_exception(TSRMLS_C);
}

PHP_METHOD(OpenCV_Mat, save) {
    opencv_mat_object *mat_object;
    zval *mat_zval = NULL;
    char *filename;
    int filename_len, cast_mode;
	bool status;
    long mode = 0;

    PHP_OPENCV_ERROR_HANDLING();
    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|l", &mat_zval, opencv_ce_cvmat, &filename, &filename_len, &mode) == FAILURE) {
        PHP_OPENCV_RESTORE_ERRORS();
        return;
    }
    PHP_OPENCV_RESTORE_ERRORS();

    mat_object = opencv_mat_object_get(getThis() TSRMLS_CC);
    cast_mode = mode;
    status = imwrite(filename, *mat_object->cvptr);
    php_opencv_throw_exception(TSRMLS_C);

    if (!status) {
        zend_throw_exception(opencv_ce_cvexception, "Failed to save image", 0 TSRMLS_CC);
    }
}

/* {{{ opencv_mat_methods[] */
const zend_function_entry opencv_mat_methods[] = { 
    PHP_ME(OpenCV_Mat, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(OpenCV_Mat, load, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(OpenCV_Mat, save, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(opencv_mat)
{
	zend_class_entry ce;
	
    INIT_NS_CLASS_ENTRY(ce, "OpenCV", "Mat", opencv_mat_methods);
	opencv_ce_cvmat = zend_register_internal_class(&ce TSRMLS_CC);

	return SUCCESS;
}
/* }}} */


