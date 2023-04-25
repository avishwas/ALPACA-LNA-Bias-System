.PHONY: view clean

view: notes.pdf
	zathura notes.pdf

notes.pdf: notes.tex
	pdflatex -synctex=1 -interaction=nonstopmode --shell-escape notes.tex > pdfbuild.log

clean:
	rm -rf notes.aux notes.fls notes.out notes.synctex.gz notes.toc _minted-notes \
		notes.fdb_latexmk notes.log notes.pdf:Zone.Identifier \
		pdfbuild.log
