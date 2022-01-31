/*
 * scheduler.h
 *
 * Author : Alexy
 */

#ifndef EDH_PROC_H_
#define EDH_PROC_H_

#include <stdint.h>
#include "listener.h"

/**
 * @def NOMBRE_MAX_TACHES
 *
 * Nombre maximum de tâches pouvant exister sur le système
 * (utilisé en interne pour allouer les structures de données)
 */
#define MAX_TASKS_NUMBER (8)

typedef enum{
  pol_EDF = 0,
  pol_EDH_ASAP, //As Soon As Possible
  pol_EDH_ALAP  //As Late As Possible
}dynamic_policy;

extern int Scheduler_create_task(Input_task input_task[MAX_TASKS_NUMBER],int nb);

extern void Scheduler_set_policy(dynamic_policy policy);
extern dynamic_policy Scheduler_get_policy();

#endif /* EDH_PROC_H_ */
