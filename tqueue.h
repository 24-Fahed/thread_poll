#ifndef __TQUEUE_H__
#define __TQUEUE_H__

#include <stdlib.h>
#include <string.h>
#include "user_errres.h"

// tqueue error 
#define TQUEUE_FULL -11
#define TQUEUE_EMPTY -12


typedef void *(*thread_func_t)(void *);

struct tqueue {
	thread_func_t func;
	void *arg;
};

typedef struct tqueue tqueue_Node;

struct tqueue_struct {
	tqueue_Node *queue;
	int head;
	int rear;
	int size;
};

typedef struct tqueue_struct tqueue_t;

int tqueue_empty(tqueue_t *tqueue);

int tqueue_full(tqueue_t *tqueue);

int tqueue_create(tqueue_t **tqueue, int max_task);

int tqueue_addNode(tqueue_t *tqueue, thread_func_t func, void *arg);

int tqueue_popNode(tqueue_t *tqueue, tqueue_Node **tnode);

int tqueue_distoryNode(tqueue_Node *tnode);

int tqueue_distoryQueue(tqueue_t * tqueue);

#endif
