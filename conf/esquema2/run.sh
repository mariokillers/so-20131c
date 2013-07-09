#!/bin/bash

# Interbloqueo entre dos personajes

BIN_DIR=../../bin
CONF_DIR=.

echo "Levantando plataforma..."
$BIN_DIR/plataforma 2>&1 &
sleep 1

echo "Levantando personajes..."
$BIN_DIR/personaje $CONF_DIR/personajeGoomba.conf& 
$BIN_DIR/personaje $CONF_DIR/personajeMario.conf& 
$BIN_DIR/personaje $CONF_DIR/personajeTortuga.conf& 

read -p "Presiona enter para matar plataforma y personajes..."

killall personaje
killall plataforma

echo "Terminado!"
