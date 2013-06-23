#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>

#include "quantum_inotify.h"
#include "plataforma.h"


/* quantum_inotify.c
 * Hilo que detecta cambios en el archivo de quantum (pasado por parametro)
 * y modifica la variable global de quantum. En caso de error, loggea y
 * setea el quantum a QUANTUM_DEFAULT.
 *
 * El hilo termina si hay un error irrecuperable o si el semaforo de plataforma
 * se libera.
 *
 * El formato del archivo de configuracion del quantum es:
 * 		quantum=n
 */
void *monitorear_quantum(void *filename)
{
	quantum_config_filename = (char *)filename;
	quantum_config = NULL;
	int msgs_length, i;
	char inotify_buffer[QUANTUM_IN_BUF_LEN];	// Buffer de mensajes
	int fd;										// File descriptor
	int wd;										// Watch descriptor

	fd = inotify_init();

	if (fd < 0) {
		log_error(Logger, "FATAL: No se pudo crear un FD de inotify! (algo salio muy mal)\n");
		exit(EXIT_FAILURE);
	}

	// Lectura inicial del archivo de configuracion
	wd = inotify_add_watch(fd, (char *)filename, IN_CLOSE_WRITE | IN_DELETE_SELF | IN_MOVE_SELF);

	if (wd >= 0) {
		actualizar_quantum(quantum_config);
	} else {
		log_error(Logger, "No se pudo leer el archivo de configuracion de quantum! No se podra modificar en runtime.");
		set_default_quantum();
		return NULL;
	}

	// Monitorear eventos todo el tiempo
	while (plataforma_activo(&mutex_plataforma)) {
		i = 0;
		// Leer los eventos del buffer (bloqueante)
		msgs_length = read(fd, inotify_buffer, QUANTUM_IN_BUF_LEN);

		// Manejar los eventos leidos
		while (i < msgs_length) {
			struct inotify_event *event = (struct inotify_event *) & inotify_buffer[i];
			if (event->mask & IN_CLOSE_WRITE) {
				log_info(Logger, "Archivo de quantum modificado");
				actualizar_quantum(quantum_config);
			}
			if (event->mask & IN_DELETE_SELF) {
				log_warning(Logger, "Archivo de quantum eliminado! No se podra modificar mas en runtime.");
				goto cerrar;
			}
			if (event->mask & IN_MOVE_SELF) {
				log_warning(Logger, "Archivo de quantum movido! No se podra modificar mas en runtime.");
				goto cerrar;
			}
	
			i += QUANTUM_IN_EVENT_SIZE + event->len; // Pasar al siguiente mensaje
		}
	}
	
cerrar:
	log_info(Logger, "Hilo monitor de quantum terminando.");
	inotify_rm_watch(fd, wd);
	close(fd);
	return NULL;
}

/* Devuelve true si plataforma sigue activo, y false si libero el semaforo
 * correspondiente para que el hilo termine correctamente
 */
bool plataforma_activo(pthread_mutex_t *mutex)
{
	if (pthread_mutex_trylock(mutex) == 0) {
		pthread_mutex_unlock(mutex);
		return false;
	} else {
		return true;
	}
}

/* Setea el valor del quantum a QUANTUM_DEFAULT y loggea un warning */
void set_default_quantum(void)
{
	quantum_inicial = QUANTUM_DEFAULT;
	log_warning(Logger, "Valor de quantum seteado a default (%d)", QUANTUM_DEFAULT);
}

/* Lee el nuevo valor de quantum del archivo de configuracion y lo actualiza.
 * Loggea y setea a QUANTUM_DEFAULT si hay un error.
 */
void actualizar_quantum()
{
	int quantum_leido;

	// Borrar config si ya se creo, para actualizarlo
	if (quantum_config != NULL) {
		config_destroy(quantum_config);
	}
	quantum_config = config_create(quantum_config_filename);

	if (config_has_property(quantum_config, QUANTUM_CONF_KEY)) {
		quantum_leido = config_get_int_value(quantum_config, QUANTUM_CONF_KEY);
	} else {
		log_warning(Logger, "Archivo de configuracion de quantum invalido");
		set_default_quantum();
		return;
	}

	if (quantum_leido > 0) {
		quantum_inicial = quantum_leido;
		log_info(Logger, "Quantum actualizado (%d)", quantum_leido);
	} else {
		log_warning(Logger, "Valor de quantum invalido");
		set_default_quantum();
	}
}
