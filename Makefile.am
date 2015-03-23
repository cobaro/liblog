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
else
	echo "No packaging configured"
endif

EXTRA_DIST = \
	bootstrap \
	LICENSE.txt \
	README.rst
