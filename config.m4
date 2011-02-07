dnl
dnl $ Id: opencv 1.0.1$
dnl

PHP_ARG_WITH(opencv, for OpenCV support,
[  --with-opencv            Enable OpenCV support], yes)

if test "$PHP_OPENCV" != "no"; then
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_OPENCV"

  AC_MSG_CHECKING(PHP version)
  AC_TRY_COMPILE([#include <php_version.h>], [
#if PHP_VERSION_ID < 50200
#error  this extension requires at least PHP version 5.2.0
#endif
],
[AC_MSG_RESULT(ok)],
[AC_MSG_ERROR([need at least PHP 5.2.0])])

  export CPPFLAGS="$OLD_CPPFLAGS"

  PHP_REQUIRE_CXX()
  PHP_SUBST(OPENCV_SHARED_LIBADD)
  AC_DEFINE(HAVE_OPENCV, 1, [ ])

  PHP_NEW_EXTENSION(opencv, opencv.c opencv_error.c opencv_arr.c, $ext_shared)

  EXT_OPENCV_HEADERS="php_opencv_api.h"

  ifdef([PHP_INSTALL_HEADERS], [
    PHP_INSTALL_HEADERS(ext/opencv, $EXT_OPENCV_HEADERS)
  ])

  if test "$PHP_OPENCV" != "no"; then
      OPENCV_CHECK_DIR=$PHP_OPENCV
      OPENCV_TEST_FILE=/include/opencv.h
      OPENCV_LIBNAME=opencv
  fi
  condition="$OPENCV_CHECK_DIR$OPENCV_TEST_FILE"

  if test -r $condition; then
   OPENCV_DIR=$OPENCV_CHECK_DIR
     CFLAGS="$CFLAGS -I$OPENCV_DIR/include"
   LDFLAGS=`$OPENCV_DIR/bin/opencv-config --libs`
  else
    AC_MSG_CHECKING(for pkg-config)
  
    if test ! -f "$PKG_CONFIG"; then
      PKG_CONFIG=`which pkg-config`
    fi

      if test -f "$PKG_CONFIG"; then
        AC_MSG_RESULT(found)
        AC_MSG_CHECKING(for opencv)
    
        if $PKG_CONFIG --exists opencv; then
            if $PKG_CONFIG --atleast-version=2.1.0 opencv; then
                opencv_version_full=`$PKG_CONFIG --modversion opencv`
                AC_MSG_RESULT([found $opencv_version_full])
                OPENCV_LIBS="$LDFLAGS `$PKG_CONFIG --libs opencv`"
                OPENCV_INCS="$CFLAGS `$PKG_CONFIG --cflags-only-I opencv`"
                PHP_EVAL_INCLINE($OPENCV_INCS)
                PHP_EVAL_LIBLINE($OPENCV_LIBS, OPENCV_SHARED_LIBADD)
                AC_DEFINE(HAVE_OPENCV, 1, [whether opencv exists in the system])
            else
                AC_MSG_RESULT(too old)
                AC_MSG_ERROR(Ooops ! You need at least opencv 2.1.0)
            fi
        else
            AC_MSG_RESULT(not found)
            AC_MSG_ERROR(Ooops ! no opencv detected in the system)
        fi
      else
        AC_MSG_RESULT(not found)
        AC_MSG_ERROR(Ooops ! no pkg-config found .... )
      fi
   fi
fi
