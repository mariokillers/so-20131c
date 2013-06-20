#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>

#define EVENT_SIZE  (sizeof (struct inotify_event))
#define EVENT_BUF_LEN     (1024 * (EVENT_SIZE + 16))

int main(void)
{
	// Crear el file descriptor para inotify
	int msgs_length, i;
	char inotify_buffer[EVENT_BUF_LEN];
	int quantum_fd = inotify_init();

	if (quantum_fd < 0) {
		fprintf(stderr, "ERROR: No se pudo crear un FD de inotify para quantum\n");
		exit(EXIT_FAILURE);
	}

	// Notificar cuando el archivo se modifica
	int quantum_wd = inotify_add_watch(quantum_fd, "./quantum.conf", IN_MODIFY);

	if (quantum_wd < 0) {
		fprintf(stderr, "ERROR: No se pudo abrir el archivo de quantum\n");
		exit(EXIT_FAILURE);
	}

	// Leer los eventos del buffer
	msgs_length = read(quantum_fd, inotify_buffer, EVENT_BUF_LEN);

	// Manejar los eventos leidos
	while (i < msgs_length) {
		struct inotify_event *event = (struct inotify_event *) & inotify_buffer[i];
		if (event->mask & IN_MODIFY) {
			printf("Me estan tocando el archivo de quantum\n");
		}
		i++;
	}
	
	// Cerrar todo
	inotify_rm_watch(quantum_fd, quantum_wd);
	close(quantum_fd);

	return EXIT_SUCCESS;
}
