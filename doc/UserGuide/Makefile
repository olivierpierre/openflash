all: UserGuide.tex
	pdflatex UserGuide
	pdflatex UserGuide
	make clean
	evince UserGuide.pdf &
	
clean:
	rm -rf *.aux *.bbl *.log *.toc *.out
	
spellcheck:
	@aspell -c 0_TitleIntro.tex -l en_us --add-tex-command "lstlisting op"
	@aspell -c 1_Characterization.tex -l en_us --add-tex-command "lstlisting op"
	@aspell -c 2_UsingTheAPI.tex -l en_us --add-tex-command "lstlisting op"
	@aspell -c 2.5_OperationSending.tex -l en_us --add-tex-command "lstlisting op"
	
