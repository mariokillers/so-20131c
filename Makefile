all: libs

libs:
	cd lib/commons; make
	cd lib/memoria; make
