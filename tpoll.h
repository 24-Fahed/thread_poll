#ifndef __TPOLL_H__
#define __TPOLL_H__

#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include "tqueue.h"

#define TPOLL_TASK_OCCUPIED -21
#define TPOLL_THREAD_OVER_LIMIT -22
#define TPOLL_THREAD_UNKNOW_ERROR -23
#define TPOLL_THREAD_FULL -24
#define TPOLL_THREAD_EMPTY -25

struct tpoll_struct {
	// 锁和条件变量
	pthread_mutex_t tpoll_mutex;
	pthread_mutex_t tpoll_read_mutex;
	pthread_cond_t task_not_empty;
	pthread_cond_t task_not_full;

	// 所有线程号和管理线程
	pthread_t *thread_list;
	pthread_t manager_thread;

	// 队列记录（任务）
	tqueue_t *task_queue;
	tqueue_Node *taskNext;
	int tqueueLen;

	// 线程对象
	int shutdown;
	int thread_min_cnt;
	int thread_max_cnt;
	int thread_curr_cnt;
	int thread_busy_cnt;
	int thread_waitExit_cnt;
	int thread_distory_cnt;
};

typedef struct tpoll_struct tpoll_t;

int tpoll_create(tpoll_t **tpoll, int threadMin, int threadMax, tqueue_t **tqueue, int taskMax);

int tpoll_addTask(tpoll_t *tpoll, thread_func_t func, void *arg);

int tpoll_popTask(tpoll_t *tpoll);

int tpoll_increaseThread(tpoll_t *tpoll, int num);

int tpoll_decreaseThread(tpoll_t *tpoll, int num);

int tpoll_distoryTpoll(tpoll_t *tpoll);

int tpoll_findThread(tpoll_t *tpoll, pthread_t tid);

void *tpoll_routine(void *arg);

void *tpoll_manager(void *arg);

#endif
