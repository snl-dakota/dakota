## Process this file with automake to produce 'Makefile.in'.

## Paths
THEORY_PATH = $(top_srcdir)/../docs/latex-theory

# LaTeX sources and dependencies for the User's Manual.
THEORY_SOURCES = Theory_Opt_SurrBased.tex \
		 Theory_Opt_UnderUnc.tex \
		 Theory_UQ_Reliability.tex \
		 Theory_UQ_StochExp.tex \
		 Theory_UQ_Epistemic.tex \
		 Theory_Parallel.tex \
		 Theory.bib

THEORY_EPS = images/filter.eps \
	     images/isogrid_N2_q6.eps \
	     images/tau_updates.eps

THEORY_PNG = $(THEORY_EPS:.eps=.png)

#THEORY_INPUTS = $(THEORY_TEST_INPUTS) $(THEORY_GENERIC_INPUTS)

pdf-local: Theory_Main.tex $(THEORY_SOURCES) $(THEORY_PNG) #$(THEORY_INPUTS)
	-rm -f $(basename $<).log
	-$(pdflatex) $<
	-$(bibtex) $(basename $<) 
	-$(pdflatex) $<
	-@latex_count=5 ; \
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' $(basename $<).log && [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "*** Re-running pdfLaTeX to resolve references ***" ;\
	      $(pdflatex) $< ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done

ps-local: Theory_Main.ps

Theory_Main.ps: Theory_Main.dvi
	-$(dvips) -o $@ $<

Theory_Main.dvi: dvi-local

dvi-local: Theory_Main.tex $(THEORY_SOURCES) $(THEORY_EPS) #$(THEORY_INPUTS)
	-rm -f $(basename $<).log
	-$(latex) $<
	-$(bibtex) $(basename $<) 
	-$(latex) $<
	-@latex_count=5 ; \
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' $(basename $<).log && [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "*** Re-running LaTeX to resolve references ***" ;\
	      $(latex) $< ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done


## Anywhere there's a chance of extracting two or more tests from the
## same input file, disable parallel make.
#.NOTPARALLEL: $(THEORY_TEST_INPUTS)

CLEANFILES = *.aux *.bbl *.blg *.brf *.dvi *.idx *.log *.out *.pdf *.ps *.toc
clean-local:
	-rm -f $(THEORY_INPUTS)

## Files to be removed on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## These files need to be distributed.
EXTRA_DIST = $(THEORY_SOURCES) bigbox.sty $(THEORY_EPS) $(THEORY_PNG)
