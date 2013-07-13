#!/bin/bash

# Interbloqueo entre dos personajes

BIN_DIR=../../bin
CONF_DIR=.

echo "Levantando personajes..."
$BIN_DIR/personaje $CONF_DIR/personajeGoomba.config& 
$BIN_DIR/personaje $CONF_DIR/personajeMario.config& 
$BIN_DIR/personaje $CONF_DIR/personajeTortuga.config& 

read -p "Presiona enter para matar personajes..."

killall -9 personaje

echo "Terminado!"
