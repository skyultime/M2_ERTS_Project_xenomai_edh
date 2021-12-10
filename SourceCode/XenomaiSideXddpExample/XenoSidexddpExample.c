#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>

#include <alchemy/task.h>
#include <alchemy/pipe.h>

static RT_PIPE my_pipe;
static int FLAG = 0;

void catch_signal(int sig){
  FLAG = 1;
}

static void task(void *arg)
{
  char buff[128];

  //Make the task periodic with a specified loop period
  rt_task_set_periodic(NULL, TM_NOW, 1000000000);

 //Start the task loop
  while(!FLAG){

	  if ( rt_pipe_read( &my_pipe, buff, 22, TM_NONBLOCK) >= 0 )
	  {
	      rt_printf("Reading message from nRT:%s\n",buff);
	  }else{
	    //rt_printf("No message to read\n");
	  }

	  rt_task_wait_period(NULL);
   }  
}

int main (void)
{
  RT_TASK task_desc;
  
  /* disable memory swap */
  if ( mlockall( MCL_CURRENT | MCL_FUTURE ) != 0 )
  {
    rt_printf("mlockall error\n");
    return 1;
  }

  signal(SIGINT,catch_signal);
   
  rt_pipe_delete(&my_pipe);
  if ( rt_pipe_create( &my_pipe, "rtp0", P_MINOR_AUTO, 0 ) != 0 )
  {
    rt_printf("rt_pipe_create error\n");
    return 1;
  }else{
    rt_printf("RT pipe create OK\n");
  }

  if (rt_task_spawn( &task_desc,  /* task descriptor */
                     "my task",   /* name */
                      0           /* 0 = default stack size */,
                      99          /* priority */,
                      T_JOINABLE, /* needed to call rt_task_join after */
                      &task,      /* entry point (function pointer) */
                      NULL        /* function argument */ )!=0)
  {
    printf("rt_task_spawn error\n");
    return 1;
  }

  //Wait for Ctrl ^ c
  pause(); //Waiting signal to occur

  /* wait for task function termination */
  rt_task_join(&task_desc);
  rt_task_delete(&task_desc);

  rt_printf("Destroy RT pipe...\n");
  rt_pipe_delete(&my_pipe);
  
  return 0;
}
