#ifndef QUANTUM_INOTIFY_H_
#define QUANTUM_INOTIFY_H_

#include <commons/config.h>

#define QUANTUM_IN_EVENT_SIZE  (sizeof (struct inotify_event)) // inotify event size
#define QUANTUM_IN_BUF_LEN     (1024 * (QUANTUM_IN_EVENT_SIZE + 16)) // inotify max buffer size
#define QUANTUM_DEFAULT 3
#define QUANTUM_CONF_KEY "quantum"

t_config *quantum_config;
char *quantum_config_filename;

void *monitorear_quantum(void *filename);
bool plataforma_activo(pthread_mutex_t *mutex);
void set_default_quantum(void);
void actualizar_quantum();

#endif
