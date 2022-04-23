all:
	cd src && $(MAKE) all

clean:
	cd src && $(MAKE) clean

check:
	cd src && $(MAKE) check

manual:
	cd doc/manual/latex && $(MAKE) all
	cp doc/manual/manual.html doc/manual/index.html
