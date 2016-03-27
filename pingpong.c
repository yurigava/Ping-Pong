#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <malloc.h>
#include "pingpong.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define MIN -20				// Prioridade de menor valor (máxima)
#define MAX 20				// Prioridade de maio valor (mínima)

task_t tMain;		//Task da Main
task_t *tAtual;		//Task atual
task_t *userTasks=NULL;	//Lista de Tasks
task_t *dispatcher;	//Tarefa do dipatcher

void pingpong_init()
{
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);
	tMain.tid = 0;
	getcontext(&(tMain.tContext));
	tAtual = &tMain;
	dispatcher=(task_t *) malloc(sizeof(task_t));
	task_create(dispatcher, dispatcher_body, "");
	#ifdef DEBUG
	printf("pingpong_init criou tarefa main (%d)\n", tMain.tid);
	#endif
}

task_t * scheduler()
{
	if(userTasks != NULL)
	{
		task_t *menorPrio=userTasks->next;
		task_t *aux=userTasks->next;
		aux = aux->next;
		do
		{
			if((menorPrio->dinPrio > aux->dinPrio) && (aux != dispatcher))
			{
				menorPrio = aux;
			}
			aux = aux->next;
		} while(aux != userTasks);
		do
		{
			aux = aux->next;
			if((aux != menorPrio) && (aux != dispatcher))
			{
				aux->dinPrio--;
				if(aux->dinPrio < -20)
				{
					aux->dinPrio = MIN;
				}
			}
		} while (aux != userTasks);
		return menorPrio;
	}
	else
	{
		return 0;
	}
}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg)
{
	char *stack;

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
	queue_append((queue_t **) &userTasks, (queue_t *)task);
	if(userTasks == NULL)
	{
		task->tid = 1;
	}
	else
	{
		task->tid = task->prev->tid + 1;
	}
	task->statPrio = 0;
	task->dinPrio = 0;
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
	task_t *aux;
	queue_remove((queue_t **) &userTasks, (queue_t *) tAtual);
	switch(exitCode)
	{
		case 0:			//Saída normal
			aux = dispatcher;
			break;
		case 1:			//Saída do Dispatcher
			free(dispatcher);
			aux = &tMain;
			break;
	}
	task_switch(aux);
	#ifdef DEBUG
	printf("task_exit terminou a tarefa %d\n", tAtual->tid);
	#endif
}

int task_getprio (task_t *task)
{
	#ifdef DEBUG
	printf("task_getprio retornou a prioridade da tarefa %d", tAtual->tid);
	#endif
	if(task != NULL)
	{
		return task->statPrio;
	}
	else
	{
		return tAtual->statPrio;
	}
}

int task_id ()
{
	#ifdef DEBUG
	printf("task_id retornou o id da tarefa %d", tAtual->tid);
	#endif
	return tAtual->tid;
}

void task_resume (task_t *task)
{
}

void task_setprio (task_t *task, int prio)
{
	int tempPrio=0;
	if(prio <= MAX)
	{
		if(prio >= MIN)
		{
			tempPrio = prio;
		}
		else
		{
			tempPrio = MIN;
		}
	}
	else
	{
		tempPrio = MAX;
	}
	if(task != NULL)
	{
		#ifdef DEBUG
		printf("task_setprio setou a prioridade da tarefa %d", task->tid);
		#endif
		task->statPrio = tempPrio;
		task->dinPrio = tempPrio;
	}
	else
	{
		#ifdef DEBUG
		printf("task_setprio setou a prioridade da tarefa %d", tAtual->tid);
		#endif
		tAtual->statPrio = tempPrio;
		tAtual->dinPrio = tempPrio;
	}
}

void task_suspend (task_t *task, task_t **queue)
{
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

void task_yield()
{
	task_switch(dispatcher);
	#ifdef DEBUG
	printf("task_yield suspendeu a execução da tarefa %d", tAtual->tid);
	#endif
}

void dispatcher_body() // dispatcher é uma tarefa
{
	task_t *next;
	while ( queue_size((queue_t *) userTasks) > 1 )
	{
		next = scheduler() ;  // scheduler é uma função
		if (next)
		{
			// ações antes de lançar a tarefa "next", se houverem
			task_switch (next) ; // transfere controle para a tarefa "next"
			// ações após retornar da tarefa "next", se houverem
		}
	}
	task_exit(1) ; // encerra a tarefa dispatcher
}
