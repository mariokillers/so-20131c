#!/bin/bash

ips=$@

if [ $# -lt 1 ]
then
	echo "Pone bien los args"
	exit 1
fi

# generar el key
ssh-keygen

for ip in $ips
do
	# mandar a cada ip
	ssh-copy-id -i ~/.ssh/id_rsa.pub $ip
done
