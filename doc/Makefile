DOC=proj_doc
PPT=overview

PDF=$(DOC).pdf $(PPT).pdf

.PHONY: all clean $(PDF)

all:
	$(MAKE) clean
	$(MAKE) $(DOC).pdf
	$(MAKE) $(PPT).pdf

$(DOC).pdf:
	pdflatex $(DOC)
	bibtex $(DOC)
	pdflatex $(DOC)
	pdflatex $(DOC)

$(PPT).pdf:
	pdflatex $(PPT)
	pdflatex $(PPT)

clean:
	rm -f *.dvi *.log *.blg *.bbl *.toc *.aux *.out *.nav *.snm *.vrb #$(PDF)
