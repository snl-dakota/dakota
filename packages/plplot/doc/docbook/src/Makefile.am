# doc/docbook/src/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Rafael Laboissiere
# Copyright (C) 2002, 2003, 2004  Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

MASTER_XML = plplotdoc.xml
MASTER_XML_IN = $(MASTER_XML).in

SOURCE_FILES = \
  advanced.xml \
  api.xml \
  api-c.xml \
  api-compat.xml \
  api-fortran.xml \
  api-obsolete.xml \
  bibliography.xml \
  c.xml \
  deploying.xml \
  fortran.xml \
  cplus.xml \
  drivers.xml \
  gui.xml \
  inline-html.ent \
  inline-print.ent \
  intro.xml \
  os_notes.xml \
  perl.xml \
  python.xml \
  simple.xml \
  tcl.xml \
  wish.xml \
  math.ent

INCLUDED_FILES =     \
  plplot-symbols.txt \
  plplot-structs.txt

STYLESHEET_HTML = plplotdoc-html.dsl
STYLESHEET_PRINT = plplotdoc-print.dsl
STYLESHEETS_IN = \
  $(STYLESHEET_HTML).in \
  $(STYLESHEET_PRINT).in

if enable_docbook
built_files = \
  $(INFO_MANIFEST) $(HTML_MANIFEST) $(MAN_MANIFEST) \
  $(BASE).jadetex \
  $(html_DATA) $(print_DATA) $(man3_MANS) $(geninfo_DATA) \
  plplotdoc.txml plplotdoc.xml-info plplotdoc.texi \
  $(INCLUDED_FILES)
endif

EXTRA_DIST = \
  $(SOURCE_FILES) \
  jadetex.cfg stylesheet.css $(built_files) \
  inline-info.ent pdftex.map

TARBALL = tarball.tgz
HTML_TARBALL = $(BASE_HTML).tar.gz
INFO_TARBALL = $(BASE_INFO).tar.gz
MAN_TARBALL = $(BASE_MAN).tar.gz
INDEX_HTML = ../www/index.html


CLEANFILES = \
  *.aux inline.ent *.log *.eps TMP.* *-tmp *.out \
  $(TARBALL) $(HTML_TARBALL) $(INFO_TARBALL) $(MAN_TARBALL)

MAINTAINERCLEANFILES = \
  $(MASTER_XML) $(STYLESHEET_HTML) $(STYLESHEET_PRINT) $(built_files) \
  Makefile.in

DISTCLEANFILES = $(JADELOG) $(INDEX_HTML)


if enable_docbook

# These required so that "wildcard" lists made using $(shell ls ...) will
# automatically work.

noinst_DATA = $(HTML_MANIFEST) $(MAN_MANIFEST) $(INFO_MANIFEST)

htmldir = $(prefix)/share/doc/$(PACKAGE)/html
html_DATA = $(shell ls *.html)

printdir = $(prefix)/share/doc/$(PACKAGE)
print_DATA = $(BASE).dvi $(BASE).ps.gz $(BASE).pdf

man3_MANS = $(shell ls *.$(MANVOL))

geninfodir = $(infodir)
geninfo_DATA = $(shell ls *.info*)

MAX_TEX_RECURSION = 10

BINDIR = ../bin

if docbook_build

# The following files are included in api-compat.sml

plplot-symbols.txt: $(BINDIR)/get-library-symbols.pl                  \
	            $(top_srcdir)/include/plplot.h
	( cd $(top_builddir)/lib ; make )
	( cd $(top_builddir)/libltdl ; make )	
	( cd $(top_builddir)/src ; make )
	$(PERL) $(BINDIR)/get-library-symbols.pl > plplot-symbols.txt \
	  $(top_builddir)/src/.libs/libplplot$(LIB_TAG).so            \
          $(top_srcdir)/include/plplot.h 3
	  
plplot-structs.txt: $(BINDIR)/get-library-structs.pl \
	            $(top_srcdir)/include/plplot.h 
	$(PERL) $(BINDIR)/get-library-structs.pl > plplot-structs.txt \
	  $(top_srcdir)/include/plplot.h

# N.B. $(STYLESHEET_HTML) assures that manifest of generated html files is
# output to the $(HTML_MANIFEST) file.
$(HTML_MANIFEST): $(SOURCE_FILES) $(MASTER_XML) $(STYLESHEET_HTML) \
                  $(INCLUDED_FILES) inline-html.ent
	ln -fs inline-html.ent inline.ent
	$(OPENJADE) $(SGML_CATALOGS) -d $(STYLESHEET_HTML) -t sgml \
	  $(XML_DECL) $(MASTER_XML)
	$(PERL) -pi -e 's/&#13;//g' `cat $(HTML_MANIFEST)`

%.ps.gz: %.dvi
	$(DVIPS) -f $< | gzip -c > $@

$(BASE).dvi: $(BASE).jadetex jadetex.cfg
	-$(JADETEX) $<
	if ! cmp $(BASE).aux prior-dvi.aux 2>/dev/null && \
	   expr $(MAKELEVEL) '<' $(MAX_TEX_RECURSION); then \
	        cp -pf $(BASE).aux prior-dvi.aux ; \
		rm -f $@ ; \
		$(MAKE) $@ ; \
	fi

$(BASE).pdf: $(BASE).jadetex jadetex.cfg
	-$(PDFJADETEX) $<
	if ! cmp $(BASE).aux prior-pdf.aux 2>/dev/null && \
	   expr $(MAKELEVEL) '<' $(MAX_TEX_RECURSION); then \
	        cp -pf $(BASE).aux prior-pdf.aux ; \
		rm -f $@ ; \
		$(MAKE) $@ ; \
	fi


$(BASE).jadetex: $(SOURCE_FILES) $(MASTER_XML) $(STYLESHEET_PRINT) \
		 $(INCLUDED_FILES) inline-print.ent
	ln -fs inline-print.ent inline.ent
	$(OPENJADE) $(SGML_CATALOGS) -d $(STYLESHEET_PRINT) -t tex \
	  -o $@ $(XML_DECL) $(MASTER_XML)


INFO_TMPDIR = info

$(INFO_MANIFEST) plplotdoc.info: plplotdoc.texi
	rm -rf $(INFO_TMPDIR)
	mkdir -p $(INFO_TMPDIR)
	-$(MAKEINFO) --force -o $(INFO_TMPDIR)/plplotdoc.info $<
	( cd $(INFO_TMPDIR) ; ls ) | cat > $(INFO_MANIFEST)
	mv $(INFO_TMPDIR)/* .
	rmdir $(INFO_TMPDIR)

plplotdoc.texi: plplotdoc.txml
	$(DB2X_TEXIXML) plplotdoc.txml

plplotdoc.txml: plplotdoc.xml-info
	$(DB2X_XSLTPROC) -s texi -o plplotdoc.txml plplotdoc.xml-info

plplotdoc.xml-info: $(BINDIR)/info-clean.pl $(SOURCE_FILES) $(MASTER_XML) \
		    $(INCLUDED_FILES) inline-info.ent
	ln -fs inline-info.ent inline.ent
	$(PERL) $(BINDIR)/info-clean.pl plplotdoc.xml > $@

$(MAN_MANIFEST): $(BINDIR)/api2man.pl $(MASTER_XML) api.xml
	$(PERL) $(BINDIR)/api2man.pl $(MASTER_XML) api.xml $(MANVOL) > $@

endif # docbook_build

validate: plplotdoc.xml
if has_onsgmls
	@ln -fs inline-html.ent inline.ent
	@echo Validating $<:
	@$(ONSGMLS) $(XML_DECL) $< > /dev/null
	@echo "No problems found."
else
	@echo "onsgmls not found: cannot validate $<"
endif

# Everything below here has to do with www-install.

# Append @ if WWW_USER non-empty
WWW_USERAT = $(shell if test "$(WWW_USER)" ; then echo $(WWW_USER)@ ; fi)

RSH_COMMAND = $(RSH) $(WWW_USERAT)$(WWW_HOST)
RCP_PREFIX = $(shell if test -z "$(WWW_USERAT)$(WWW_HOST)" ; then \
                       echo "" ; \
                     else \
                       echo "$(WWW_USERAT)$(WWW_HOST):" ; \
                     fi)

$(HTML_TARBALL): $(HTML_MANIFEST) stylesheet.css
	rm -rf $(BASE_HTML)
	mkdir $(BASE_HTML)
	cp `cat $(HTML_MANIFEST)` $(BASE_HTML)
	cp stylesheet.css $(BASE_HTML)
	tar cfz $(HTML_TARBALL) $(BASE_HTML)
	rm -rf $(BASE_HTML)

$(INFO_TARBALL): $(INFO_MANIFEST)
	rm -rf $(BASE_INFO)
	mkdir $(BASE_INFO)
	cp `cat $(INFO_MANIFEST)` $(BASE_INFO)
	tar cfz $(INFO_TARBALL) $(BASE_INFO)
	rm -rf $(BASE_INFO)

$(MAN_TARBALL): $(MAN_MANIFEST)
	rm -rf $(BASE_MAN)
	mkdir $(BASE_MAN)
	cp `cat $(MAN_MANIFEST)` $(BASE_MAN)
	tar cfz $(MAN_TARBALL) $(BASE_MAN)
	rm -rf $(BASE_MAN)

# size-href.pl requires as input (for size) every file mentioned as
# an href in $(INDEX_HTML).in with a @SIZE@ attribute.  Make the
# prerequisite list consistent with this requirement.

$(INDEX_HTML): $(BINDIR)/size-href.pl $(INDEX_HTML).in \
	       $(BASE).ps.gz $(BASE).pdf $(BASE).dvi \
	       $(INFO_TARBALL) $(MAN_TARBALL) $(HTML_TARBALL)
	$(PERL) $(BINDIR)/size-href.pl < $(INDEX_HTML).in > $@

$(TARBALL): $(INDEX_HTML) \
	    $(BASE).ps.gz $(BASE).pdf $(BASE).dvi \
	    $(INFO_TARBALL) $(MAN_TARBALL) $(HTML_TARBALL)
	tar cfz $(TARBALL) $(HTML_TARBALL) \
                $(INFO_TARBALL) $(MAN_TARBALL) \
                $(BASE).ps.gz $(BASE).pdf $(BASE).dvi \
		-C .. COPYING -C www index.html

www-uninstall:
	$(RSH_COMMAND) rm -rf $(WWW_DIR)

www-install: $(TARBALL) www-uninstall
	$(RSH_COMMAND) mkdir -p $(WWW_DIR)
	$(RCP) $(TARBALL) $(RCP_PREFIX)$(WWW_DIR)
	$(RSH_COMMAND) tar xfz $(WWW_DIR)/$(TARBALL) -C $(WWW_DIR)
	$(RSH_COMMAND) tar xfz $(WWW_DIR)/$(HTML_TARBALL) -C $(WWW_DIR)
	$(RSH_COMMAND) chgrp -R $(WWW_GROUP) $(WWW_DIR)
	$(RSH_COMMAND) chmod -R g=u $(WWW_DIR)
	$(RSH_COMMAND) rm -f $(WWW_DIR)/$(TARBALL)

.PHONY: www-uninstall www-install

# enable_docbook
endif
