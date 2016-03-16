#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <malloc.h>
#include "pingpong.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */

int tid_count=0;
task_t tMain;

void pingpong_init()
{
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);
	t_Main.tid = 0;
	tid_count++
	getcontext(&(tMain.tContext));
}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg)
{
	char *stack;

	task->tid = tid_count;
	tid_count++;
	getcontext(&(task->tContext));
	stack = malloc(STACKSIZE);
	if (stack)
	{
		task->tContext.uc_stack.ss_sp = stack;
		task->tContext.uc_stack.ss_size = STACKSIZE;
		task->tContext.uc_stack.ss_flags = 0;
		task->tContext.uc_link = 0;
	}
	else
	{
	  perror ("Erro na criação da pilha");
	  exit (1);
	}
	makecontext(&(task->tContext), start_routine, 1, arg);
	#ifdef DEBUG
	printf("task_create criou tarefa %d\n", task->tid);
	#endif
}


int task_id ()
{
	return 
}
