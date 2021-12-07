#include "listener.h"
#include "loop_task.h"

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
	printf("\nUse EDF alg.? Enter Y or N: \n");
	scanf(" %c", &answer);
	
	switch (answer){
	
	case 'Y':
	case 'y':
	    Scheduler_set_policy(DYNAMIC);
	    printf("dynamic priority\n");
	    break;

	case 'N':
	case 'n':
	    Scheduler_set_policy(FIXED);
	    printf("fixed priority\n");
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

		printf("\nEnter period: "); 
		if(scanf("%d", &my_input_task[i].period)< 0){
			printf("Incorrect input, exiting\n");
			exit(-1);
		}
		assert(my_input_task[i].period > 0
		&& my_input_task[i].period < MAX_PERIOD
		&& "Forbidden value");

		if(Scheduler_get_policy() == DYNAMIC){
                  printf("\nEnter deadline: "); 
		  if(scanf("%d", &my_input_task[i].deadline)< 0){
		    printf("Incorrect input, exiting\n");
		    exit(-1);
		  }
		  
                  assert(my_input_task[i].deadline > 0 
		  && my_input_task[i].deadline < MAX_DEADLINE 
		  && "Forbidden value");
                
		}else{
                  printf("\nEnter priority: "); 
		  if(scanf("%d", &my_input_task[i].priority)< 0){
		    printf("Incorrect input, exiting\n");
		    exit(-1);
		  }
		
                  assert(my_input_task[i].priority >= 0 
		  && my_input_task[i].priority < MAX_PRIORITY 
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
