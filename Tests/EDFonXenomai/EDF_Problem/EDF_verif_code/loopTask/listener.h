/**
 * @brief Fonctions utilitaires pour récupérer les infos des tâches
 * 
 */

#if !defined __LISTENER_H_
#define __LISTENER_H_

/**
 * @struct Msg_log
 * @brief défini comment est composé un message reçu par socket.
 */
typedef struct{
	int period;
	int deadline;
	#if 0 
          int execution_time;
        #endif
        int priority;
} Input_task;

/**
 * @brief initialisation du listener
 *
 * Doit être appelée avant toute autre fonction de ce module
 */
void Listener_init (void);
extern Input_task* Listener_ask_inputs(int *tasks_number);

#define MAX_INPUT_TASKS     (8)

#define MAX_PERIOD          (1000000000)
#define MAX_ACTIVATION_TIME (1000000000)
#define MAX_DEADLINE        (1000000000)

#define MAX_PRIORITY        (100) //[0-99]

#endif /* __IHM_H_ */
