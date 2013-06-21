#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>

#define EVENT_SIZE  (sizeof (struct inotify_event))
#define EVENT_BUF_LEN     (1024 * (EVENT_SIZE + 16))

/* inotify_test.c
 * Monitorea un archivo pasado por parametro y emite un mensaje
 * cuando es modificado o borrado
 */
int main(int argc, char *argv[])
{
	int msgs_length, i;
	char inotify_buffer[EVENT_BUF_LEN];	// Buffer de mensajes
	int fd;								// File descriptor
	int wd;								// Watch descriptor

	if (argc < 2) {
		fprintf(stderr, "ERROR: No se paso ningun archivo por parametro\n");
		exit(EXIT_FAILURE);
	}

	fd = inotify_init();

	if (fd < 0) {
		fprintf(stderr, "ERROR: No se pudo crear un FD de inotify! (algo salio muy mal)\n");
		exit(EXIT_FAILURE);
	}

	// Notificar cuando el archivo se modifica, crea o elimina
	wd = inotify_add_watch(fd, argv[1], IN_CLOSE_WRITE | IN_DELETE_SELF);

	if (wd < 0) {
		fprintf(stderr, "ERROR: No se pudo abrir el archivo!\n");
		exit(EXIT_FAILURE);
	}

	while (1) { // TODO: Usar epoll?
		i = 0;
		// Leer los eventos del buffer
		msgs_length = read(fd, inotify_buffer, EVENT_BUF_LEN);

		// Manejar los eventos leidos
		while (i < msgs_length) {
			struct inotify_event *event = (struct inotify_event *) & inotify_buffer[i];
			if (event->mask & IN_CLOSE_WRITE) {
				printf("Archivo modificado\n");
			}
			else if (event->mask & IN_DELETE_SELF) {
				printf("Archivo eliminado. No se van a recibir mas notificaciones!\n");
				goto cerrar;
			}
			i += EVENT_SIZE + event->len; // Pasar al siguiente mensaje
		}
	}
	
	// Cerrar todo
cerrar:
	inotify_rm_watch(fd, wd);
	close(fd);

	return EXIT_SUCCESS;
}
