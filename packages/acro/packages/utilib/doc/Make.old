##
## Makefile for documentation, using the doxygen tool
##

tex-user:
	@echo "#"
	@echo "# tex-user"
	@echo "#"
	@(mkdir -p latex)
	doxygen Doxyfile-user.latex > latex/doxygen.out 2>&1
	@cat latex/refman.tex | awk 'BEGIN {flag=1;}\
		{if ($$1 == "END-OF-DOC") flag=0;\
		if (flag == 1) print $$0;\
		}' > latex/userman.tex 2>&1
	@cp uguide/uguide.tex latex
	@(cd latex; rm -f myoxygen.sty; ln -f -s ../uguide/myoxygen.sty .)

tex-ref:
	@echo "#"
	@echo "# tex-ref"
	@echo "#"
	@mkdir -p latex-ref
	doxygen Doxyfile-ref.latex > latex-ref/doxygen.out 2>&1
	@(cd latex-ref;\
	cat refman.tex | awk 'BEGIN {flag=1;}\
		{if (($$2 == "Page") && ($$3 == "Documentation}")) flag=0;\
		if ($$1 == "\\printindex") flag=1;\
		if (($$2 == "Page") && ($$3 == "Index}")) flag=0;\
		if (($$2 == "Class") && ($$3 == "Documentation}")) flag=1;\
		if (flag == 1) print $$0;\
		}' > tmp.tex 2>&1;\
	mv tmp.tex refman.tex;\
	rm -f myoxgeyn.sty; ln -f -s ../uguide/myoxygen.sty .)

ps: tex-user tex-ref
	(cd latex;\
	latex userman.tex;\
	bibtex userman;\
	latex userman.tex;\
	latex userman.tex;\
	dvips -o -K userman.dvi;\
	mv userman.ps ..) #> ps-user.out 2>1
	(cd latex-ref;\
	latex refman.tex;\
	bibtex refman;\
	makeindex refman.idx;\
	latex refman.tex;\
	latex refman.tex;\
	dvips -o -K refman.dvi;\
	mv refman.ps ..;) #> ps-ref.out 2>1
	make clean

pdf: tex-user #tex-ref
	@echo "#"
	@echo "# tex-pdf"
	@echo "#"
	@echo "#"
	@echo "# latexing in the 'latex' directory"
	@echo "#"
	(cd latex;\
	pdflatex userman.tex;\
	bibtex userman;\
	pdflatex userman.tex;\
	pdflatex userman.tex;\
	mv userman.pdf ..) #> pdf-user.out 2>1
	#(cd latex-ref;\
	#latex refman.tex;\
	#bibtex refman;\
	#makeindex refman.idx;\
	#latex refman.tex;\
	#pdflatex refman.tex;\
	#mv refman.pdf ..;) #> pdf-ref.out 2>1
	make clean

html:	web

web:
	doxygen Doxyfile-ref.html > html-ref/doxygen.out 2>&1
	doxygen Doxyfile-user.html > html/doxygen.out 2>&1
	html/installdox -l utilib-ref.tag@../html-ref html/*.html

clean:
	$(RM) -Rf latex latex-ref man/man3

all: pdf web
	gzip -f userman.pdf
	
