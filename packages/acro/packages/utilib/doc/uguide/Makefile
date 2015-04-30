##
## Makefile for documentation, using the doxygen tool
##

tex:
	@echo "#"
	@echo "# tex"
	@echo "#"
	@(mkdir -p latex)
	doxygen Doxyfile.latex > latex/doxygen.out 2>&1
	cp DOEbwlogo.pdf SANDreport.cls snllineblk.pdf latex
	cp refman.tex latex/userman.tex
	cp abstract.tex latex
	cp doxygen.sty latex
	@(for file in latex/*.tex ; do		\
		echo $${file};\
		sed -i -e 's/subsection/section/' $${file};\
	done;)
	@(for file in latex/arrayPage.tex latex/llPage.tex latex/heapPage.tex latex/htPage.tex latex/setPage.tex latex/splayPage.tex latex/mpiPage.tex latex/factoryPage.tex latex/mpePage.tex latex/packPage.tex latex/commonioPage.tex ; do	\
		sed -i -e 's/section{/subsection{/' $${file};\
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
	cp userman.ps ..) #> ps-user.out 2>1
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
	cp userman.pdf ..) #> pdf-user.out 2>1
	#$(MAKE) clean

html:	web

rtf:
	@(mkdir -p rtf)
	doxygen Doxyfile.rtf > rtf/doxygen.out 2>&1

web:
	@(mkdir -p html)
	doxygen Doxyfile.html > html/doxygen.out 2>&1
#	html/installdox -l utilib-ref.tag@../html-ref html/*.html

clean:
	$(RM) -rf html latex man/man3

all: pdf web
	gzip -f userman.pdf

