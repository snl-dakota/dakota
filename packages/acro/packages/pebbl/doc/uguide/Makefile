
all: doc-pdf

#  This is no longer built by default, but can be made with "make doc-ps"
doc-ps:
	cd 1.4.1; \
	latex user-guide; \
	bibtex user-guide; \
	latex user-guide; \
	latex user-guide; \
	dvips user-guide.dvi -o -K; \
	mv user-guide.ps ..

doc-pdf: 
	cd 1.4.1; \
	pdflatex user-guide; \
	bibtex user-guide; \
	pdflatex user-guide; \
	pdflatex user-guide; \
	mv user-guide.pdf ..

html:

clean:
	rm -f user-guide.ps user-guide.pdf; \
	cd 1.4.1; \
	rm -f user-guide.bbl user-guide.log user-guide.aux user-guide.toc; \
	rm -f user-guide.blg user-guide.dvi



