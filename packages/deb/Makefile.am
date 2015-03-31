# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

EXTRA_DIST = changelog.in control.in rules.in

.PHONY: clean deb

deb: rules control changelog
	ln -nsf ./packages/deb @top_srcdir@/debian
	(cd @top_srcdir@; fakeroot $(MAKE) -f debian/rules binary)

edit = sed \
	-e 's,@PACKAGE\@,$(PACKAGE),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@PACKAGE_CONFIG_OPTS\@,$(PACKAGE_CONFIG_OPTS),g'

changelog: changelog.in
	rm -f changelog changelog.tmp
	$(edit) $(srcdir)/changelog.in >changelog.tmp
	mv changelog.tmp changelog

control: control.in
	rm -f control control.tmp
	$(edit) $(srcdir)/control.in >control.tmp
	mv control.tmp control

rules: rules.in
	rm -f rules rules.tmp
	$(edit) $(srcdir)/rules.in >rules.tmp
	mv rules.tmp rules



