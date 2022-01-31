#include "EDH_proc.h"
#include "listener.h"
#include <stdlib.h>

int main (void)
{

	int tasks_nb;
	Listener_init();

	Input_task *my_input_task = (Input_task *)malloc(sizeof(Input_task) * MAX_INPUT_TASKS);
	my_input_task = Listener_ask_inputs(&tasks_nb);

	if (Scheduler_create_task(my_input_task,tasks_nb)){
          exit(-1);	
	}

	while (1);

	return 0;	
}
