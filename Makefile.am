# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	lib \
	doc \
	m4 \
	packages \
	script \
	test

package: dist
if RUN_RPMBUILD
	(cd packages/rpm; make rpm)
endif
if RUN_DEBUILD
	(tar zxf $(PACKAGE)-$(VERSION).tar.gz && \
	cd $(PACKAGE)-$(VERSION) && \
	./bootstrap && \
	CFLAGS="$(PACKAGE_CFLAGS)" ./configure $(PACKAGE_CONFIGURE) \
	    --prefix=/usr && \
	cd packages/deb && \
	make deb && \
	rm -rf $(PACKAGE)-$(VERSION))
endif

doc_DATA = \
	LICENSE.txt

EXTRA_DIST = \
	$(doc_DATA) \
	bootstrap \
	README.rst
