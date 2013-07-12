#!/bin/bash

ARCHIVO=$1
IPS=${@:2}

if [ $# -lt 2 ]
then
	echo "Pone bien los argumentos"
	exit 1
fi

# generar el key
ssh-keygen

for ip in $ips
do
	# mandar a cada ip
	ssh-copy-id -i ~/.ssh/id_rsa.pub $ip
done

for ip in $IPS
do
	# Si ya existe, borrar todo
	ssh $ip "rm -rf ~/repo*"
	# Copiar a hosts
	scp $ARCHIVO $ip:~
	scp lib_paths.txt $ip:~
	# Extraer a ~/repo
	ssh $ip "tar xvf $ARCHIVO; mv sisop* repo"
	# Setear variables de entorno en host
	ssh $ip "cat ~/lib_paths.txt >> ~/.bashrc && rm ~/lib_paths.txt"
done
