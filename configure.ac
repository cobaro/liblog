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
PACKAGE_CFLAGS="-std=gnu99 -Wall -Werror -O2 -pipe -mtune=generic -DNDEBUG"
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
 pthread.h \
 netinet/in.h \
 stdarg.h \
 stdbool.h \
 stdio.h \
 stdint.h \
 string.h \
 syslog.h \
 sys/time.h \
 unistd.h
)
AC_HEADER_TIME

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
 lib/Makefile
 lib/libcobaro-log0/Makefile
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
