# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

.PHONY: darwin

clean-local:
	rm -f *.pkg
	rm -rf $(PACKAGE)-$(VERSION)
	rm -rf _inst

darwin:
	rm -rf _inst && \
	mkdir -p _inst/dev && \
	mkdir -p _inst/run && \
	tar zxf $(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz && \
	(cd $(PACKAGE)-$(VERSION) && \
	./bootstrap && \
	CFLAGS="$(PACKAGE_CFLAGS)" ./configure \
		--enable-shared --enable-static --prefix=/usr/local && \
	make install DESTDIR=$(abs_top_builddir)/packages/darwin/_inst/dev) && \
	mkdir -p _inst/run/usr/local/lib && \
	mv _inst/dev/usr/local/lib/libcobaro-log0*dylib _inst/run/usr/local/lib && \
	pkgbuild \
		--root _inst/dev \
		--install-location "/" \
		--identifier "org.cobaro.log.dev" \
		--version $(VERSION) \
		"$(PACKAGE)-dev.pkg" && \
	pkgbuild \
		--root _inst/run \
		--install-location "/" \
		--identifier "org.cobaro.log.run" \
		--version $(VERSION) \
		"$(PACKAGE)-run.pkg" && \
	productbuild \
		--distribution $(srcdir)/dist.plist \
		--package-path $(builddir) \
		--version $(VERSION) \
		$(PACKAGE)-$(VERSION).pkg

EXTRA_DIST = \
	dist.plist

