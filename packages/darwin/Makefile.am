# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

.PHONY: darwin

clean-local:
	rm -rf $(PACKAGE)-$(VERSION)
	rm -rf _inst

darwin:
	mkdir -p _inst && \
	tar zxf $(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz && \
	(cd $(PACKAGE)-$(VERSION) && \
	./bootstrap && \
	CFLAGS="$(PACKAGE_CFLAGS)" ./configure \
		--disable-shared --prefix=/usr/local && \
	make install DESTDIR=$(abs_top_builddir)/packages/darwin/_inst) && \
	pkgbuild \
		--root _inst \
		--identifier org.cobaro.log \
		--version $(VERSION) \
		"$(PACKAGE).pkg"
