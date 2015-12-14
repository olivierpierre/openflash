all: PpcModels.tex
	pdflatex PpcModels
	pdflatex PpcModels
	make clean
	evince PpcModels.pdf &
	
clean:
	rm -rf *.aux *.bbl *.log *.toc *.out
	
spellcheck:
	@aspell -c PpcModels.tex -l en_us --add-tex-command "lstlisting op"
	
