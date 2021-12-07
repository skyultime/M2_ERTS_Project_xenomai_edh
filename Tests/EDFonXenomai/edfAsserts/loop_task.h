/*
 * scheduler.h
 *
 * Author : Alexy
 */

#ifndef LOOPTASK_H_
#define LOOPTASK_H_

#include <stdint.h>
#include "listener.h"

/**
 * @def NOMBRE_MAX_TACHES
 *
 * Nombre maximum de tâches pouvant exister sur le système
 * (utilisé en interne pour allouer les structures de données)
 */
#define MAX_TASKS_NUMBER (8)

typedef enum {
	DYNAMIC = 0, //EDF ONLY
	FIXED,
} Policy;

extern int Scheduler_create_task(Input_task input_task[MAX_TASKS_NUMBER],int nb);

extern void Scheduler_set_policy(Policy policy);
extern Policy Scheduler_get_policy();

#endif /* LOOPTASK_H_ */
