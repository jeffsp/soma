all:
	qmake -o Makefile.qt
	$(MAKE) -f Makefile.qt

run: all
	./usability

clean:
	$(MAKE) -f Makefile.qt clean
	rm -f Makefile.qt usability
