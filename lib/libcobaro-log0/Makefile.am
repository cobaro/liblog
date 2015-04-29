# COPYRIGHT_BEGIN
# Copyright (C) 2015, cobaro.org
# All rights reserved.
# COPYRIGHT_END

edit = sed \
	-e s,@RELEASE\@,`date +\%Y\%m\%d`,g \
	-e 's,@PACKAGE\@,$(PACKAGE),g' \
	-e 's,@VERSION\@,$(VERSION),g'

Doxyfile: Doxyfile.in $(top_srcdir)/configure.ac
	echo $@ $<
	rm -f $@ $@.tmp
	$(edit) $< >$@.tmp
	mv $@.tmp $@

doc: $(libcobaro_log0_la_HEADERS) Doxyfile
	$(DOXYGEN) 

