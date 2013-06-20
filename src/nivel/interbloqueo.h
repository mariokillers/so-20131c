/*
 * interbloqueo.h
 *
 *  Created on: 20/06/2013
 *      Author: utnso
 */

#ifndef INTERBLOQUEO_H_
#define INTERBLOQUEO_H_

int recovery, recovery_time;
pthread_mutex_t deadlock_mutex;
pthread_t thread_interbloqueo;

#endif /* INTERBLOQUEO_H_ */
