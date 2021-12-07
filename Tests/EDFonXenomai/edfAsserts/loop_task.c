#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <stdbool.h>

#include "loop_task.h"

#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/mutex.h>

//Inspired from the code https://www.ashwinnarayan.com/post/xenomai-realtime-programming-part-2/

#define CLOCK_RES 1e-9 //Clock resolution is 1 ns by default
#define LOOP_PERIOD 1e9 //Expressed in ticks

int IsResume = 1;
static Policy current_policy;
_Bool entrance = true;

RT_TASK loop_task[MAX_TASKS_NUMBER]={};
RTIME tinit;

//create semaphore/mutex;
static RT_MUTEX mutex;

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
  rt_printf("\nStarting task %s with period of %d\n",curtaskinfo.name,per);

  //Make the task periodic with a specified loop period
  rt_task_set_periodic(NULL, TM_NOW, per);

  tstart = rt_timer_read();

  //Start the task loop
  while(1){

    #if 1
    //Acquire Mutex to prevent concurrent access for variables tstart_WCET && now    
    if(rt_mutex_acquire(&mutex, TM_INFINITE) != 0){
      rt_printf("rt_mutex_acquire ERROR\n");
    }   
    	tstart_WCET= rt_timer_read();
    	rt_printf("Begin of WCET for %s (T=%.5f ms)\n",curtaskinfo.name,(tstart_WCET - tinit)/1000000.0);

    //Release Mutex to prevent concurrent access for variables tstart_WCET && now    
    if(rt_mutex_release(&mutex) != 0){
      rt_printf("rt_mutex_release ERROR\n");
    }
    #endif

    #if defined(CPU_BURN)
      rt_timer_spin(25000000);
    #else
      for(int i=0;i<5000;i++){
        for (int j=0;j<2000;j++){
          //Dummy operations
        }
      }
    #endif
    
    #if 1
    //Acquire Mutex to prevent concurrent access for variables tstart_WCET && now    
    if(rt_mutex_acquire(&mutex, TM_INFINITE) != 0){
      rt_printf("rt_mutex_acquire ERROR\n");
    }  
        rt_printf("End of WCET for %s (T=%.5f ms), WCET: %.5f ms\n",curtaskinfo.name,(rt_timer_read() - tinit)/1000000.0,(rt_timer_read()-tstart_WCET)/1000000.0);
    
    //Release Mutex to prevent concurrent access for variables tstart_WCET && now    
    if(rt_mutex_release(&mutex) != 0){
      rt_printf("rt_mutex_release ERROR\n");
    }
    #endif
    
    #if 0
      rt_printf("Loop time: %.5f ms",(rt_timer_read() - tstart)/1000000.0,);
    #endif
    
    ctr++;
    tstop=tstart;

    if(rt_task_wait_period(NULL)==0){
      rt_printf("\nPeriod reached for %s (T=%.5f ms)\n", 
              curtaskinfo.name,
             (rt_timer_read() - tinit/*tstop*/)/1000000.0);    
    }
  }
}

int Scheduler_create_task(Input_task input_task[MAX_TASKS_NUMBER],int nb)
{

  tasks_number = nb;
  assert (tasks_number < MAX_TASKS_NUMBER && "Max nb tasks is 4 !");

  cpu_set_t mask;

  // set CPU to affine to
  CPU_ZERO(&mask);    // clear all CPUs
  CPU_SET(0, &mask);    // select CPU 3

  char str[20];

  //Lock the memory to avoid memory swapping for this program
  mlockall(MCL_CURRENT | MCL_FUTURE);

  if(rt_mutex_create(&mutex,"mutex") != 0){
    rt_printf("rt_mutex_create ERROR\n");
  }  

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

    // set task cpu affinity
    rt_task_set_affinity(&loop_task[i], &mask);    

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

