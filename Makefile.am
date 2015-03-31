# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	doc \
	lib \
	m4 \
	packages \
	script \
	test

package: dist
if RUN_RPMBUILD
	(cd packages/rpm; make rpm)
endif
if RUN_DEBUILD
	(cd packages/deb; make deb)
endif

EXTRA_DIST = \
	bootstrap \
	LICENSE.txt \
	README.rst
