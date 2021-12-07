#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <math.h>

#include "loop_task.h"

//Inspired from the code https://www.ashwinnarayan.com/post/xenomai-realtime-programming-part-2/

#define CLOCK_RES 1e-9 //Clock resolution is 1 ns by default
#define LOOP_PERIOD 1e9 //Expressed in ticks

int IsResume = 1;
static Policy current_policy;

RT_TASK loop_task[MAX_TASKS_NUMBER]={};
RTIME tinit;

/**
 * Compteur du nombre de tâches créées (utilisé essentiellement pour contrôler
 * qu'on ne dépasse pas la limite)
 */
static uint32_t tasks_number;

void loop_task_proc(void *arg)
{
  RT_TASK *curtask;
  RT_TASK_INFO curtaskinfo;
  int iret = 0;

  RTIME tstart,tstop,tstart_WCET,now;
  

  curtask = rt_task_self();
  rt_task_inquire(curtask, &curtaskinfo);
  int ctr = 0;

  int per = * (int *) arg;

  //Print the info
  rt_printf("Starting task %s with period of %d ....\n",curtaskinfo.name,per);

  //Make the task periodic with a specified loop period
  rt_task_set_periodic(NULL, TM_NOW, per);

  tstart = rt_timer_read();

  //Start the task loop
  while(1){

    for(int i=0;i<5000;i++){
      for (int j=0;j<5000;j++){
        //Dummy operations
        if (i == j && j== 0)
          tstart_WCET= rt_timer_read();
        if (i == j && j== 1000-1)
          now = rt_timer_read();
      }
    }

    rt_printf("Task name:%s, Loop count: %d, Loop time: %.5f ms, WCET: %.5f ms\n", curtaskinfo.name, ctr,(rt_timer_read() - tstart)/1000000.0,(now-tstart_WCET)/1000000.0);
    
    ctr++;
    tstop=tstart;

    #if 0
      if(strcmp(curtaskinfo.name,"loop_task_0")==0 && IsResume == 1){
        for(int i=1;i<tasks_number;i++){
          rt_task_resume(&loop_task[i]);
          IsResume = 0;
        }
      }    
    #endif

    if(rt_task_wait_period(NULL)==0){
      rt_printf("Period reached for task :%s ( %.5f ms)\n", 
              curtaskinfo.name,
             (rt_timer_read() - tinit/*tstop*/)/1000000.0);    
    }
    
  }
}

int Scheduler_create_task(Input_task input_task[MAX_TASKS_NUMBER],int nb)
{

  tasks_number = nb;
  assert (tasks_number < MAX_TASKS_NUMBER && "Max nb tasks is 4 !");

  char str[20];

  //Lock the memory to avoid memory swapping for this program
  mlockall(MCL_CURRENT | MCL_FUTURE);
  
  rt_printf("Creating prio and non-prio tasks (%d tasks)...\n",nb);
  tinit = rt_timer_read();  

  for(int i=0;i<nb;i++){
    sprintf(str, "loop_task_%d",i);
   
    rt_print_init(4096,str);  
    if (Scheduler_get_policy() == DYNAMIC){
        if(rt_task_create_dyna(&loop_task[i], str, 0, input_task[i].deadline, 0)!=0){
          printf("rt_task_create_dyna error\n");
          return 1;
        }
    }else{
      if(rt_task_create(&loop_task[i], str, 0, input_task[i].priority, 0)!=0){
        printf("rt_task_create error\n");
        return 1;
      }
    }    

    rt_task_start(&loop_task[i], &loop_task_proc, &input_task[i].period);

    //Since task starts in suspended mode, start task 
    #if 0
      if(i>0 && i<=tasks_number){
        rt_task_suspend(&loop_task[i]);  
    }
    #endif

    rt_task_join(&loop_task[i]); //Used only with mode T_JOINABLE
  }  

  #if 1
    //Wait for Ctrl-C
    pause();
  #endif

  return 0;
}

/****************************************************************************************/
/****************************************************************************************/

void Scheduler_set_policy(Policy policy){
	current_policy = policy;
}

Policy Scheduler_get_policy(){
	return current_policy;
}

