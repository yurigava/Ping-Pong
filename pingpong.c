#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <malloc.h>
#include <signal.h>
#include <sys/time.h>
#include "pingpong.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define MIN -20				// Prioridade de menor valor (máxima)
#define MAX 20				// Prioridade de maio valor (mínima)
#define ticks 20			// Tamanho do quantum padrão

task_t *tMain;				//Task da Main
task_t *tAtual;				//Task atual
task_t *ltProntas=NULL;		//Lista de Tasks
task_t *dispatcher;			//Tarefa do dipatcher

struct sigaction action;	//Tratador de sinal
struct itimerval timer;		//Timer

unsigned int totalTicks=0;

void pingpong_init()
{
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);

	//Seta tarefa principal
	tMain = (task_t *) malloc(sizeof(task_t));
	tMain->tid = 0;
	tMain->statPrio = 0;
	tMain->dinPrio = 0;
	tMain->activations = 0;
	tMain->procTime = 0;
	tMain->sys_task = false;
	getcontext(&(tMain->tContext));
	tAtual = tMain;
	queue_append((queue_t **) &ltProntas, (queue_t *)tMain);

	//Seta tarefa do dispatcher
	dispatcher=(task_t *) malloc(sizeof(task_t));
	task_create(dispatcher, dispatcher_body, "");
	dispatcher->sys_task = true;

	//Seta controlador de ticks
	action.sa_handler = ticks_body;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0 ;
	if (sigaction (SIGALRM, &action, 0) < 0)
	{
		perror ("Erro em sigaction: ");
		exit (1) ;
	}

	// ajusta valores do temporizador
	timer.it_value.tv_usec = 1000;		// primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0;			// primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000;	// disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0;		// disparos subsequentes, em segundos

	// arma o temporizador ITIMER_REAL
	if (setitimer (ITIMER_REAL, &timer, 0) < 0)
	{
		perror ("Erro em setitimer: ") ;
		exit (1) ;
	}
	#ifdef DEBUG
	printf("pingpong_init inicializou o sistema\n");
	#endif
}

task_t * scheduler()
{
	if(ltProntas != NULL)
	{
		task_t *menorPrio = ltProntas;
		task_t *aux = ltProntas->next;
		do
		{
			if((menorPrio->dinPrio > aux->dinPrio) && (aux != dispatcher))
			{
				menorPrio = aux;
			}
			aux = aux->next;
		} while(aux != ltProntas);
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
		} while (aux != ltProntas);
		menorPrio->dinPrio = menorPrio->statPrio;
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
	queue_append((queue_t **) &ltProntas, (queue_t *)task);
	if(task->next != task)	// Verifica se é a main
	{
		task->tid = task->prev->tid + 1;
	}
	task->statPrio = 0;
	task->dinPrio = 0;
	task->activations = 0;
	task->procTime = 0;
	task->sys_task = false;
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
	queue_remove((queue_t **) &ltProntas, (queue_t *) tAtual);
	printf("Task %2d exit: execution time %5u ms, processor time %5u ms, %4u activations\n", tAtual-> tid,
													systime(), tAtual->procTime, tAtual->activations);
	switch(exitCode)
	{
		case 0:			//Saída normal
			aux = dispatcher;
			break;
		case 1:			//Saída do Dispatcher
			free(dispatcher);
			aux = tMain;
			break;
	}
	#ifdef DEBUG
	printf("task_exit terminou a tarefa %d\n", tAtual->tid);
	#endif
	task_switch(aux);
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

int task_join (task_t *task)
{
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
	task->activations++;
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
	#ifdef DEBUG
	printf("task_yield suspendeu a execução da tarefa %d", tAtual->tid);
	#endif
	task_switch(dispatcher);
}

unsigned int systime ()
{
	return totalTicks;
}

void dispatcher_body() // dispatcher é uma tarefa
{
	task_t *next;
	while ( queue_size((queue_t *) ltProntas) > 1 )
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

void ticks_body(int signum)
{
	totalTicks++;
	tAtual->procTime++;
	if(tAtual->sys_task)
	{
		return;
	}
	if(tAtual->quantum == -1)
	{
		tAtual->quantum = ticks;
	}
	else if(tAtual->quantum == 0)
	{
		tAtual->quantum--;
		task_yield();
	}
	else
	{
		tAtual->quantum--;
	}
}
