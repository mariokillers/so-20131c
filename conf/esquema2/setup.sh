#!/bin/bash

if [ $# -ne 2 ]
then
	echo "Se necesitan 2 hosts para este esquema!"
	exit 1
fi

# Empezar de 0
ssh $1 "cd ~/mario-killers/conf/esquema2 && rm nivel\ DeadLock.config plataforma.config"
ssh $2 "cd ~/mario-killers/conf/esquema1 && rm personajeTortuga.config personajeLuigi.config personajeGoomba.config personajeMario.config"

# VM1
scp nivel\ DeadLock.config plataforma.config $1:~/mario-killers/conf/esquema2	
ssh $1 "cd ~/mario-killers/conf/esquema2 && 
	echo -e 'orquestador=$1:5000\n' >> plataforma.config &&
	echo miDireccion=`hostname -I | cut -d' ' -f1`:20010 >> nivel\ DeadLock.config"

# VM2
scp personajeTortuga.config personajeLuigi.config personajeGoomba.config personajeMario.config $2:~/mario-killers/conf/esquema2
ssh $2 "cd ~/mario-killers/conf/esquema2 &&
	echo -e 'orquestador=$1:5000\n' >> personajeTortuga.config
	echo -e 'orquestador=$1:5000\n' >> personajeLuigi.config
	echo -e 'orquestador=$1:5000\n' >> personajeGoomba.config
	echo -e 'orquestador=$1:5000\n' >> personajeMario.config"
