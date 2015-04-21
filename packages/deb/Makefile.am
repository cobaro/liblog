# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END


.PHONY: clean deb

deb: rules control changelog
	ln -nsf ./packages/deb @top_srcdir@/debian
	(cd @top_srcdir@; fakeroot $(MAKE) -f debian/rules binary)

edit = sed \
	-e 's,@PACKAGE\@,$(PACKAGE),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@PACKAGE_CONFIGURE\@,$(PACKAGE_CONFIGURE),g' \
	-e 's,@PACKAGE_CFLAGS\@,$(PACKAGE_CFLAGS),g'

changelog: changelog.in
	$(edit) $(srcdir)/changelog.in >changelog

control: control.in
	$(edit) $(srcdir)/control.in >control

rules: rules.in
	$(edit) $(srcdir)/rules.in >rules

EXTRA_DIST = \
    changelog.in \
    control.in \
    rules.in \
    compat
