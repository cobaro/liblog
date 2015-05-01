# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

libcobaro_log0dir = $(includedir)/libcobaro-log0
libcobaro_log0_HEADERS = \
	log.h

# Doxygen requires a little work to avoid enumerating files and the
# autochain isn't particularly set up for it.
#
# Note:
## we need to use DESTDIR for use in packaging and be compliant
## with make DESTDIR=
##
## The chmod/mkdir which works around the distcheck target being done
## internally with install and read only directories
##
## We use a stamp as a target rather than a doc_DATA as that would
## cause [de]installation to happen twice on any file mentioned there
##
## Doxygen might not be installed so we test for RUN_DOXYGEN from configure

htmldocdir = $(docdir)/Doxygen

.doxygen_stamp: Doxyfile $(libcobaro_log0_HEADERS)
	$(DOXYGEN)
	touch $@

all-local: .doxygen_stamp

clean-local:
	rm -rf html latex .doxygen_stamp

install-data-hook:
if RUN_DOXYGEN
	rm -rf $(DESTDIR)$(htmldocdir)
	mkdir -p $(DESTDIR)$(docdir)
	chmod +w $(DESTDIR)$(docdir)
	cp -r html $(DESTDIR)$(htmldocdir)
endif

uninstall-hook:
if RUN_DOXYGEN
	rm -rf $(DESTDIR)$(htmldocdir)
endif

EXTRA_DIST = \
	Doxyfile.in
