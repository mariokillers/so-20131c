tp-20131c-mario-killers
=======================

Implementacion del TP de Sistemas Operativos del grupo Mario Killers

## Directorios
* `bin`: Binarios ejecutables, proceso koopa

* `lib`: Binarios de bibliotecas compartidas + código fuente

* `doc`: Cualquier tipo de documentación

* `src`: Código fuente propio

* `conf`: Archivos de configuración

## Cómo ejecutar

Hay que setear las variables de entorno `LD_LIBRARY_PATH`, `LIBRARY_PATH` y `C_INCLUDE_PATH`. Para esto, usamos el comando `export`:

    export LD_LIBRARY_PATH=/home/utnso/tp-20131c-mario-killers/lib
    export LIBRARY_PATH=/home/utnso/tp-20131c-mario-killers/lib
    export C_INCLUDE_PATH=/home/utnso/tp-20131c-mario-killers/lib

Para automatizar esto, podemos usar el archivo `~./bashrc` así se ejecutan los comandos cada vez que abren una terminal:

    echo "export LD_LIBRARY_PATH=/home/utnso/tp-20131c-mario-killers/lib" >> ~/.bashrc
    echo "export LIBRARY_PATH=/home/utnso/tp-20131c-mario-killers/lib" >> ~/.bashrc
    echo "export C_INCLUDE_PATH=/home/utnso/tp-20131c-mario-killers/lib" >> ~/.bashrc
    
Para compilar, se paran en el directorio del repo y ejecutan `make` (compila todos los ejecutables y las bibliotecas). También pueden hacer:

* `make libs` (commons y libmemoria)
* `make personaje`
* `make plataforma`
* `make nivel`

Una vez que se haya compilado lo que necesitan, vayan al directorio `bin` (`cd bin`) y ejecuten cada programa en una terminal separada:

* `./plataforma`
* `./nivel ../conf/nivel1.config 20000`
* `./personaje ../conf/personaje.config`
