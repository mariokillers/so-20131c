#!/bin/bash

if [ $# -ne 4 ]
then
	echo "Se necesitan 4 hosts para este esquema!"
	exit 1
fi

# Empezar de 0
ssh $1 "cd ~/mario-killers/conf/esquema1 && rm Level1-1.config personajeMario.config"
ssh $2 "cd ~/mario-killers/conf/esquema1 && rm plataforma.config Level2-1.config personajeHongo.config"
ssh $3 "cd ~/mario-killers/conf/esquema1 && rm Level3-1.config personajeLuigi.config personajeGoomba.config"
ssh $4 "cd ~/mario-killers/conf/esquema1 && rm Level8-8.config personajeTortuga.config"

# VM1
scp Level1-1.config personajeMario.config $1:~/mario-killers/conf/esquema1
ssh $1 "cd ~/mario-killers/conf/esquema1 && 
	echo -e 'IPlocal=$2:5000\n' >> plataforma.config &&
	echo miDireccion=\`hostname -I | cut -d' ' -f1\`:20001 >> Level1-1.config &&
	echo -e 'orquestador=$2:5000\n' >> personajeMario.config"

# VM2
scp plataforma.config Level2-1.config personajeHongo.config $2:~/mario-killers/conf/esquema1
ssh $2 "cd ~/mario-killers/conf/esquema1 &&
	echo -e 'orquestador=$2:5000\n' >> Level2-1.config &&
	echo miDireccion=\`hostname -I | cut -d' ' -f1\`:20002 >> Level2-1.config &&
	echo -e 'orquestador=$2:5000\n' >> personajeHongo.config"

# VM3
scp Level3-1.config personajeLuigi.config personajeGoomba.config $3:~/mario-killers/conf/esquema1
ssh $3 "cd ~/mario-killers/conf/esquema1 &&
	echo 'orquestador=$2:5000' >> Level3-1.config &&
	echo -e miDireccion=\`hostname -I | cut -d' ' -f1\`:20003 >> Level3-1.config &&
	echo -e 'orquestador=$2:5000\n' >> personajeGoomba.config"

# VM4
scp Level8-8.config personajeTortuga.config $4:~/mario-killers/conf/esquema1
ssh $4 "cd ~/mario-killers/conf/esquema1 &&
	echo -e 'orquestador=$2:5000\n' >> Level8-8.config &&
	echo -e miDireccion=\`hostname -I | cut -d' ' -f1\`:20008 >> Level8-8.config &&
	echo -e 'orquestador=$2:5000\n' >> personajeTortuga.config"
