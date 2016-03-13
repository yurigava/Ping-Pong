#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila

void queue_append (queue_t **queue, queue_t *elem)
{
	if(elem == NULL)
	{
		printf("Erro! Elemento Vazio.\n");
		return;
	}
	if((elem->next != NULL) || (elem->prev != NULL))
	{
		printf("Erro! Este elemento já está em uma fila.\n");
		return;
	}
	if(queue == NULL)
	{
		printf("Erro! Lista não existe.\n");
		return;
	}
	if((*queue) == NULL) //Lista vazia
	{
		(*queue) = elem;
		elem->prev = elem;
		elem->next = elem;
	}
	else
	{
		elem->next = (*queue);
		elem->prev = (*queue)->prev;
		(*queue)->prev->next =  elem;
		(*queue)->prev = elem;
	}
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro

queue_t *queue_remove (queue_t **queue, queue_t *elem)
{
	if(elem == NULL)
	{
		printf("Erro! Elemento Vazio.\n");
		return NULL;
	}
	if(queue == NULL)
	{
		printf("Erro! Lista não existe.\n");
		return NULL;
	}
	if((*queue) != elem) //Não é o primeiro da lista
	{
		queue_t *aux=(*queue)->next;
		while((aux != (*queue)) && (aux != elem))
		{
			aux = aux->next;
		}
		if(aux != elem)
		{
			printf("Erro! O elemento não pertence à esta lista\n");
			return NULL;
		}
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}
	else
	{
		if(elem->next != elem) // Não é o último elemento restante
		{
			(*queue) = elem->next;
			elem->prev->next = elem->next;
			elem->next->prev = elem->prev;
		}
		else
		{
			(*queue)=NULL;
		}
	}
	elem->next=NULL;
	elem->prev=NULL;
	return elem;
}

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue)
{
	int i = 1;
	if(queue == NULL)
	{
		return 0;
	}
	queue_t *aux = queue->next;
	while(aux != queue)
	{
		aux = aux->next;
		i++;
	}
	return i;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*))
{
	queue_t *aux = queue;
	if(queue == NULL)
	{
		printf("%s []\n",name);
		return;
	}
	printf("%s: [",name);
	do
	{
		print_elem(aux);
		printf(" ");
		aux = aux->next;
	}
	while(aux != queue);
	printf("]\n");
}
