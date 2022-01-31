#include "listener.h"
#include "EDH_proc.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <assert.h>

void Listener_init (void){
	if(system("clear")!= 0){
		exit(-1);
	}
	#if 0
	  system("stty -icanon min 1 time 0 -echo");
	#endif
}

extern Input_task* Listener_ask_inputs(int32_t *tasks_number){

	fflush(stdout);
	
	char answer;
	printf("\nChoose Policy ? Enter 0 (EDF),1 (EDH_ASAP) or 2 (EDH_ALAP): \n");
	scanf(" %d", &answer);
	
	switch (answer){

	case 0:
	    Scheduler_set_policy(pol_EDF);
	    printf("EDF priority\n");
	    break;
	
	case 1:
	    Scheduler_set_policy(pol_EDH_ASAP);
	    printf("EDH_ASAP policy\n");
	    break;

	case 2:
	    Scheduler_set_policy(pol_EDH_ALAP);
	    printf("EDH_ALAP priority\n");
	    break;

        default:
            printf("Wrong answer, exiting\n");
            exit(-1);

	}
	
	printf("\nEnter the number of tasks: ");
	if(scanf("%d",tasks_number)< 0){
		printf("Incorrect input, exiting\n");
		exit(-1);
	}
	assert(*tasks_number >1 && *tasks_number <= MAX_INPUT_TASKS && "invalid value,exiting\n");

	Input_task *my_input_task = (Input_task *)malloc(sizeof(Input_task) * MAX_INPUT_TASKS);

	for(int i=0;i<(*tasks_number);i++){
		printf("\n************************ Task %d ************************\n",i+1);

		printf("\nEnter WCET: "); 
		if(scanf("%d", &my_input_task[i].period)< 0){
			printf("Incorrect input, exiting\n");
			exit(-1);
		}
		assert(my_input_task[i].period > 0
		&& my_input_task[i].period < MAX_PERIOD
		&& "Forbidden value");

		
                  printf("\nEnter deadline: "); 
		  if(scanf("%d", &my_input_task[i].deadline)< 0){
		    printf("Incorrect input, exiting\n");
		    exit(-1);
		  }
		  
                  assert(my_input_task[i].deadline > 0 
		  && my_input_task[i].deadline < MAX_DEADLINE 
		  && "Forbidden value");

		if(Scheduler_get_policy() != pol_EDF){
                  printf("\nEnter WCEC: "); 
		  if(scanf("%d", &my_input_task[i].WCEC)< 0){
		    printf("Incorrect input, exiting\n");
		    exit(-1);
		  }
		  
                  assert(my_input_task[i].WCEC > 0 
		  && my_input_task[i].WCEC < MAX_WCEC 
		  && "Forbidden value");
                
		}

		#if 0 
                  printf("\nEnter activation time: "); 
		  if(scanf("%d",&my_input_task[i].execution_time)< 0){
			printf("Incorrect input, exiting\n");
			exit(-1);
		  }
		  assert(my_input_task[i].execution_time > 0 
		  && my_input_task[i].execution_time < MAX_DEADLINE
		  && "Forbidden value");
		#endif
	}

    return my_input_task;
}
