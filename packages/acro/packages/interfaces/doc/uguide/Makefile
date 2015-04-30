##
## Makefile for documentation, using the doxygen tool
##

tex:
	@echo "#"
	@echo "# tex"
	@echo "#"
	@(mkdir -p latex)
	doxygen Doxyfile.latex > latex/doxygen.out 2>&1
	cat latex/refman.tex | awk 'BEGIN {flag=1;}\
		{if ($$1 == "END-OF-DOC") flag=0;\
		if (flag == 1) print $$0;\
		}' > latex/userman.tex 2>&1
	@(for file in latex/*.tex ; do		\
		echo $${file};\
		sed -i -e 's/subsection/section/' $${file};\
	done;)
	@(cd latex; rm -f myoxygen.sty; ln -f -s ../myoxygen.sty .)

ps: tex
	@echo "#"
	@echo "# ps"
	@echo "#"
	(cd latex;\
	latex userman.tex;\
	bibtex userman;\
	latex userman.tex;\
	latex userman.tex;\
	dvips -o -K userman.dvi;\
	mv userman.ps ..) #> ps-user.out 2>1
	$(MAKE) clean

pdf: tex
	@echo "#"
	@echo "# pdf"
	@echo "#"
	(cd latex;\
	pdflatex userman.tex;\
	bibtex userman;\
	pdflatex userman.tex;\
	pdflatex userman.tex;\
	mv userman.pdf ..) #> pdf-user.out 2>1
	$(MAKE) clean

html:	web

web:
	@(mkdir -p html)
	doxygen Doxyfile.html > html/doxygen.out 2>&1
#	html/installdox -l utilib-ref.tag@../html-ref html/*.html

clean:
	$(RM) -rf html latex man/man3

all: pdf web
	gzip -f userman.pdf

