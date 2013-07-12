#!/bin/bash

ARCHIVO=$1
DESTINO=$2
IPS=${@:3}

if [$# -lt 3]
then
	echo "Pone bien los argumentos"
	exit 1
fi

for ip in $IPS
do
	scp $ARCHIVO $ip:$DESTINO
done
