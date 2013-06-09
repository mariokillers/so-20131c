all: libs plataforma personaje nivel

libs:
	cd lib/commons; make
	cd lib/memoria; make

personaje:
	cd src/personaje; make

nivel:
	cd src/nivel; make

plataforma:
	cd src/plataforma; make
