# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

SUBDIRS = \
	SPECS \
	SOURCES

.PHONY: rpmsetup

rpmsetup:
	(cd SOURCES; $(MAKE) rpm)
	(cd SPECS; $(MAKE) spec)

rpm: rpmsetup @top_srcdir@/@PACKAGE@-@VERSION@.tar.gz
	rpmbuild --define "_topdir `pwd`" -ba SPECS/spec

