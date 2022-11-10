#include "tqueue.h"

int tqueue_full(tqueue_t *tqueue) {
	return (tqueue->rear + 1) % (tqueue->size) == (tqueue->head) ? 1 : 0;
}

int tqueue_empty(tqueue_t *tqueue)  {
	return (tqueue->head + 1) % (tqueue->size) == (tqueue->rear) ? 1: 0;
}

// use array build a queue
int tqueue_create(tqueue_t **tqueue_ptr, int max_task) {
	// create queue array with max member
	*tqueue_ptr = (tqueue_t *)malloc(sizeof(tqueue_t));
	if (*tqueue_ptr == NULL) {
		return NULL_POINTER; 
	}

	tqueue_t *tqueue = *tqueue_ptr;
	max_task += 2;	
	tqueue->queue = (tqueue_Node *)malloc(sizeof(tqueue_Node) * max_task);
	bzero(tqueue->queue, sizeof(tqueue_Node) * (max_task));
	tqueue->size = max_task;

	tqueue->head = 0;
	tqueue->rear = 1;
	return 0;
}

int tqueue_addNode(tqueue_t *tqueue, thread_func_t func, void *arg) {
	// judge queue exist
	if (tqueue == NULL) {
		return NULL_POINTER;
	}

	// test full stack
	if (tqueue_full(tqueue)) {
		return TQUEUE_FULL;
	}

	// create node
	// tqueue_Node *tempNode = (tqueue_Node *)malloc(sizeof(tqueue_Node));
	// if (tempNode == NULL) {
	// 	return NULL_POINTER;
	// }
	tqueue_Node *tempNode = (tqueue->queue) + (tqueue->rear);

	tempNode->func = func;
	tempNode->arg = arg;

	// tempNode = (tqueue->queue) + (tqueue->rear);
	tqueue->rear = (tqueue->rear + 1) % (tqueue->size);

	return 0;
}

int tqueue_popNode(tqueue_t *tqueue, tqueue_Node **tnode) {
	// judge queue exist
	if (tqueue == NULL) {
		return NULL_POINTER;
	}

	// test full stack
	if (tqueue_empty(tqueue)) {
		return TQUEUE_EMPTY;
	}

	// pop node
	tqueue->head = (tqueue->head + 1) % (tqueue->size);
	*tnode = tqueue->queue + (tqueue->head);
	// tqueue->head = (tqueue->head + 1) % (tqueue->size);
	if (*tnode == NULL) {
		return NULL_POINTER;
	}

	return 0;
}

int tqueue_distoryNode(tqueue_Node *tnode) {
	free(tnode);

	return 0;
}

int tqueue_distoryQueue(tqueue_t *tqueue) {
	tqueue_Node *Node = NULL;
	int ret = 0;
	free(tqueue->queue);
	return 0;
}
