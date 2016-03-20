#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <malloc.h>
#include "pingpong.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */

int tid_count=0;	//Contagem de id
task_t tMain;		//Task da Main
task_t *tAtual;		//Task atual

void pingpong_init()
{
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);
	tMain.tid = 0;
	tid_count++;
	getcontext(&(tMain.tContext));
	tAtual = &tMain;
	#ifdef DEBUG
	printf("pingpong_init criou tarefa main (%d)\n", tMain.tid);
	#endif
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
	makecontext(&(task->tContext), (void*)(*start_routine), 1, arg);
	#ifdef DEBUG
	printf("task_create criou tarefa %d\n", task->tid);
	#endif
	if(task != NULL)
	{
		return task->tid;
	}
	else
	{
		return -1;
	}
}

void task_exit (int exitCode)
{
	task_switch(&tMain);
	#ifdef DEBUG
	printf("task_exit terminou a tarefa %d\n", tAtual->tid);
	#endif
}

int task_switch (task_t *task)
{
	int resul;
	task_t *aux;
	#ifdef DEBUG
	printf("task_switch mudou %d -> %d\n", tAtual->tid, task->tid);
	#endif
	aux = tAtual;
	tAtual = task;
	resul = swapcontext(&(aux->tContext), &(task->tContext));
	if(resul != -1)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int task_id ()
{
	return tAtual->tid; 
}
