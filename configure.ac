# COPYRIGHT_BEGIN
# Copyright (C) cobaro.org
# All rights reserved.
# COPYRIGHT_END

AC_INIT([libcobaro-log0], [1.0.0], [bill@cobaro.org])

AC_CONFIG_SRCDIR(lib/libcobaro-log0/log.h)
AC_PREREQ(2.63)
AM_INIT_AUTOMAKE([-Wall, -Werror foreign])
AM_CONFIG_HEADER(lib/config.h)

m4_ifdef([AM_PROG_AR], [AM_PROG_AR]) dnl Workaround for old automake

LT_INIT()

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_PROG_LN_S
AC_PROG_INSTALL

# Following tests use C compilation/linkage.
AC_LANG(C)

# pthreads
SAVED_CFLAGS="$CFLAGS" # -Werror breaks ACX_PTHREAD
CFLAGS=""
ACX_PTHREAD()
CFLAGS="$PTHREAD_CFLAGS $SAVED_CFLAGS "
LDFLAGS="$PTHREAD_LDFLAGS $LDFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"

## Packaging
# See packages/rpm/spec.in for how this is glued together
PACKAGE_CFLAGS="-std=c99 -Wall -Werror -O2 -pipe -mtune=generic -DNDEBUG"
AC_SUBST(PACKAGE_CFLAGS)
PACKAGE_CONFIGURE="--enable-shared --enable-static"
AC_SUBST(PACKAGE_CONFIGURE)

# RPM Settings
AC_PATH_PROG([RPMBUILD], [rpmbuild], [no])
AM_CONDITIONAL(RUN_RPMBUILD, test "x$RPMBUILD" != xno)
RPM_BUILD_ROOT="`pwd`/packages/rpm/BUILD"
AC_SUBST(RPM_BUILD_ROOT)

# DEB Settings
AC_PATH_PROG([DEBUILD], [debuild], [no])
AM_CONDITIONAL(RUN_DEBUILD, test "x$DEBUILD" != xno)

# OSX Settings
AC_PATH_PROG([PKGBUILD], [pkgbuild], [no])
AM_CONDITIONAL(RUN_PKGBUILD, test "x$PKGBUILD" != xno)


# Library version numbering see:
#  http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# The rules are:
#  Before a release if any source code change, then bump REVISION
#  Interface addition only, then increment AGE
#  Interface changes, then increment CURRENT, set REVISION and AGE to 0
LIB_CURRENT=1
LIB_REVISION=0
LIB_AGE=0
LIB_VERSION=$LIB_CURRENT:$LIB_REVISION:$LIB_AGE
AC_DEFINE_UNQUOTED(LIB_VERSION, "$LIB_VERSION", [lib C:R:A])
AC_SUBST(LIB_CURRENT)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_AGE)
AC_SUBST(LIB_VERSION)

# Header files we need to know about
AC_CHECK_HEADERS(\
 arpa/inet.h \
 errno.h \
 netinet/in.h \
 pthread.h \
 stdarg.h \
 stdbool.h \
 stdio.h \
 stdint.h \
 string.h \
 syslog.h \
 sys/param.h \
 sys/time.h \
 time.h \
 unistd.h \
)
AC_HEADER_TIME

# Check for pthread_spin_init(), which is an optional feature of
# POSIX.1-2008, and not implemented by Darwin (as of version 14.3) as
# well as older earlier Linux releases.
AC_CACHE_CHECK([for pthread spinlocks], [cobaro_cv_pthread_spinlocks], [
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([
#define _XOPEN_SOURCE 700
#include <pthread.h>
        ],[
pthread_spinlock_t *lk = 0;
(void)pthread_spin_lock(lk);
        ])],
        [cobaro_cv_pthread_spinlocks=yes],
        [cobaro_cv_pthread_spinlocks=no])])
if test x"$cobaro_cv_pthread_spinlocks" = xyes ; then
    AC_DEFINE([HAVE_PTHREAD_SPINLOCKS], 1,
        [Define to 1 if pthread_spinlocks are available])
fi

# Check for GCC's atomic intrinsics.
AC_CACHE_CHECK([for gcc atomic builtins], [cobaro_cv_gcc_atomics], [
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([],
            [int value = 1; __sync_fetch_and_add(&value, 3);])],
        [cobaro_cv_gcc_atomics=yes],
        [cobaro_cv_gcc_atomics=no])])
if test x"$cobaro_cv_gcc_atomics" = xyes ; then
    AC_DEFINE([HAVE_GCC_ATOMICS], 1,
        [Define to 1 if gcc supports the __sync_x operations on this platform])
fi

# See if valgrind is available
if test "x$cross_compiling" = xyes; then
   # Don't try to run valgrind when cross-compiling, because a) it's
   # not going to be there anyway
   VALGRIND=no
else
   AC_PATH_PROG([VALGRIND], [valgrind], [no])
fi
AM_CONDITIONAL(RUN_VALGRIND, test "x$VALGRIND" != xno)

# Documentation

# See if doxygen is available
AC_PATH_PROG([DOXYGEN], [doxygen], [echo])
AM_CONDITIONAL(RUN_DOXYGEN, test "x$DOXYGEN" != xecho)

# See if sphinx-build is available
AC_PATH_PROG([SPHINXBUILD], [sphinx-build], [echo])
AM_CONDITIONAL(RUN_SPHINX_BUILD, test "x$SPHINXBUILD" != xecho)


AC_CONFIG_FILES([
 Makefile
 doc/Makefile
 doc/conf.py
 lib/Makefile
 lib/libcobaro-log0/Makefile
 lib/libcobaro-log0/Doxyfile
 m4/Makefile
 packages/Makefile
 packages/libcobaro-log0.pc
 packages/deb/Makefile
 packages/rpm/Makefile
 packages/rpm/SPECS/Makefile
 packages/rpm/SOURCES/Makefile
 script/Makefile
 test/Makefile
])

AC_OUTPUT
