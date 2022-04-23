DOCKER = docker
PANDOC=${DOCKER} run --rm -v `pwd`:/data:z docker.io/pandoc/latex --verbose
default: docs


all: manual.pdf manual.html manual.rst

clean:
	rm manual.pdf manual.html manual.rst

manual.pdf:
	${PANDOC} -o manual.pdf manual.tex -s
	mv manual.pdf ../

manual.html:
	${PANDOC} -o manual.html manual.tex -s --mathjax
	mv manual.html ../


manual.rst:
	${PANDOC} -o manual.rst manual.tex -s --mathjax
	mv manual.tex ../

