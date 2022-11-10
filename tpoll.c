#include <stdio.h>
#include <string.h>
#include <time.h>
#include "tpoll.h"

#define DEBUG 1

struct tpoll_routine {
	tpoll_t* tpoll;
	int num; // recording place in thread_list
};

typedef struct tpoll_routine tpoll_routine_parameter;

int tpoll_create(tpoll_t **tpoll_ptr, int threadMin, int threadMax, tqueue_t **tqueue_ptr, int taskMax) {
	*tpoll_ptr = (tpoll_t *)malloc(sizeof(tpoll_t));
	if (*tpoll_ptr == NULL) {
		return NULL_POINTER;
	}

	int ret = tqueue_create(tqueue_ptr, taskMax);
	if (ret != 0) {
		return ret;
	}

	tpoll_t *tpoll = *tpoll_ptr;
	tqueue_t *tqueue = *tqueue_ptr;

	tpoll->task_queue = *tqueue_ptr;
	tpoll->taskNext = NULL;
	tpoll->tqueueLen = taskMax;

	pthread_mutex_init(&(tpoll->tpoll_mutex), NULL);
	pthread_mutex_init(&(tpoll->tpoll_read_mutex), NULL);
	pthread_cond_init(&(tpoll->task_not_empty), NULL);
	pthread_cond_init(&(tpoll->task_not_full), NULL);

	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	pthread_mutex_lock(&(tpoll->tpoll_read_mutex));

	tpoll->thread_list = (pthread_t *)malloc(sizeof(pthread_t) * threadMax);
	bzero(tpoll->thread_list, sizeof(pthread_t) * threadMax);
	// tpoll_routine_parameter param = {tpoll, 0};
	tpoll_routine_parameter *param = NULL;

	for (int i = 0; i < threadMin; i++) {
		param = (tpoll_routine_parameter *)malloc(sizeof(tpoll_routine_parameter));
		param->num = i;
		param->tpoll = tpoll;

		ret = pthread_create((tpoll->thread_list + i), NULL, tpoll_routine, (void *)param);
		if (ret != 0) {
			return ret;
		}
		tpoll->thread_curr_cnt ++;
	}

	// 0.0.1 change: add thread number parameter
	
	ret = pthread_create(&(tpoll->manager_thread), NULL, tpoll_manager, (void *)tpoll);
	if (ret != 0) {
		return ret;
	}
	tpoll->shutdown = 0;
	tpoll->thread_min_cnt = threadMin;
	tpoll->thread_max_cnt = threadMax;
	tpoll->thread_distory_cnt = 0;
	tpoll->thread_busy_cnt = 0;
	tpoll->thread_waitExit_cnt = 0;
	
	pthread_mutex_unlock(&(tpoll->tpoll_read_mutex));
	pthread_mutex_unlock(&(tpoll->tpoll_mutex));
#if DEBUG
	sleep(1);
#endif
	return 0;
}

int tpoll_addTask(tpoll_t *tpoll, thread_func_t func, void *arg) {
	// pthread_mutex_lock(tpoll->tpoll_mutex);
	
	if (tpoll == NULL) {
		return NULL_POINTER;
	}
	int ret = tqueue_addNode(tpoll->task_queue, func, arg);
	if (ret != 0) {
		return ret;
	}

	// pthread_mutex_unlock(tpoll->tpoll_mutex);

	return 0;
}

int tpoll_popTask(tpoll_t *tpoll) {
	// if (tpoll->taskNext != NULL) {
	// 	return TPOLL_TASK_OCCUPIED;
	// }
	if (tpoll == NULL) {
		return NULL_POINTER;
	}

	int ret = tqueue_popNode(tpoll->task_queue, &(tpoll->taskNext));
	
	if (ret == TQUEUE_EMPTY) {
		tpoll->taskNext = NULL;
		return ret;
	}

	if (tpoll->taskNext == NULL) {
		return NULL_POINTER;
	}
	return 0;
}

int tpoll_increaseThread(tpoll_t *tpoll, int num) {
	if (tpoll->thread_curr_cnt >= tpoll->thread_max_cnt) {
		return TPOLL_THREAD_FULL;
	}
	
	if (num + tpoll->thread_curr_cnt >= tpoll->thread_max_cnt) {
		return TPOLL_THREAD_OVER_LIMIT;
	}
	int plc = 0;
	tpoll_routine_parameter *param = NULL;
	int c = 0;
	for (c = 0; c < num; c++) {
		param = (tpoll_routine_parameter*)malloc(sizeof(tpoll_routine_parameter));
		plc = tpoll_findThread(tpoll, 0);
		if (plc < 0) {
			return TPOLL_THREAD_UNKNOW_ERROR;
		}
#if DEBUG
		if (plc > tpoll->thread_max_cnt) {
			printf("add a error number :%d \n", plc);
		}
#endif
		param->tpoll = tpoll;
		param->num = plc;
		pthread_create(tpoll->thread_list + plc, NULL, tpoll_routine, (void *)param);
		tpoll->thread_curr_cnt ++;
	}
	return c;
}

int tpoll_decreaseThread(tpoll_t *tpoll, int num) {
	tpoll->thread_distory_cnt = num;
	return 0;
}

int tpoll_distoryTpoll(tpoll_t *tpoll) {
	tpoll->shutdown = 1;
	return 0;
}

int tpoll_findThread(tpoll_t *tpoll, pthread_t tid) {
	pthread_t *thread_list = tpoll->thread_list;

	int curr_cnt = tpoll->thread_max_cnt;
	
	for (int i = 0; i < curr_cnt; i++) {
		if (*(thread_list + i) == tid)
			return i;
	}
	return -1; // none mach
}

void *tpoll_routine(void *arg) {
	// tpoll_t *tpoll = (tpoll_t *)arg;
	tpoll_t *tpoll = ((tpoll_routine_parameter *)arg) -> tpoll;
	int thread_place = ((tpoll_routine_parameter*)arg) -> num;
	tqueue_Node *tnode = NULL;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	int ret = 0;
	// ts.tv_sec += 5;
	//
	free((tpoll_routine_parameter*)arg); // release
	while (1) { 
		pthread_mutex_lock(&(tpoll->tpoll_mutex));
		while (tqueue_empty(tpoll->task_queue) && tpoll->shutdown == 0) {
			// clock_gettime(CLOCK_REALTIME, &ts);
			// ts.tv_sec += 5;
			// pthread_cond_timedwait(&(tpoll->task_not_empty), &(tpoll->tpoll_mutex), &ts);
			pthread_cond_wait(&(tpoll->task_not_empty), &(tpoll->tpoll_mutex));
		}

		if (tpoll->thread_distory_cnt > 0) {
			tpoll->thread_distory_cnt --;
			tpoll->thread_waitExit_cnt ++;
			pthread_mutex_unlock(&(tpoll->tpoll_mutex));
			goto TAG_ROUTINE_EXIT;
		}

	
		ret = tpoll_popTask(tpoll);
		if (ret != 0) {
			fprintf(stderr, "pthread %ld err occor: %d \n", pthread_self(), ret);
			pthread_mutex_unlock(&(tpoll->tpoll_mutex));
			continue;
		}

		tnode = tpoll->taskNext;
		tpoll->thread_busy_cnt ++;

		pthread_mutex_unlock(&(tpoll->tpoll_mutex));

		if (tnode == NULL) {
			fprintf(stderr, "pthread ^ld NULLTASK error \n");
			continue;
		}

		tnode->func(tnode->arg);

		pthread_mutex_lock(&(tpoll->tpoll_mutex));
		tpoll->thread_busy_cnt --;
		pthread_mutex_unlock(&(tpoll->tpoll_mutex));
		
	}
	TAG_ROUTINE_EXIT:
	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	// todo: 将线程号清零
	*(tpoll->thread_list + thread_place) = 0;
	tpoll->thread_waitExit_cnt --;
	tpoll->thread_curr_cnt --;
	pthread_mutex_unlock(&(tpoll->tpoll_mutex));

	return NULL;
}

void *tpoll_manager(void *arg) {
	tpoll_t *tpoll = (tpoll_t *)arg;

	while (1) {
		// 上锁管理者锁，是一个只读锁，管理不进行信息修改的情况下获取这把锁
		pthread_mutex_lock(&(tpoll->tpoll_read_mutex));
		// 探测任务池中是否为空
		if (tpoll->shutdown == 0) {
			if (!tqueue_empty(tpoll->task_queue)) {
				pthread_cond_signal(&(tpoll->task_not_empty));
			}
			else {
				;
			}

			if (!tqueue_full(tpoll->task_queue)) {
				pthread_cond_signal(&(tpoll->task_not_full));
			}
			else {
				;
			}
		}
		pthread_mutex_unlock(&(tpoll->tpoll_read_mutex));

		if (tpoll->shutdown == 1) {
			// 回收线程
			pthread_mutex_lock(&(tpoll->tpoll_mutex));
			tpoll_decreaseThread(tpoll, tpoll->thread_curr_cnt);
			
			pthread_t *thread_list = malloc(sizeof(pthread_t) * (tpoll->thread_max_cnt));
			thread_list = memcpy(thread_list, tpoll->thread_list, sizeof(pthread_t) * (tpoll->thread_max_cnt));

			int thread_sum = tpoll->thread_max_cnt;
			pthread_mutex_unlock(&(tpoll->tpoll_mutex));

			for (int i = 0; i < thread_sum; i++) {
#if DEBUG
				printf("cycle %ld \n", *(thread_list + i));
#endif
				if (*(thread_list + i) == 0)
					continue;
				pthread_cond_broadcast(&(tpoll->task_not_empty)); // 唤醒被这个条件阻塞的线程
#if DEBUG
				printf("tid: %ld \n", *(thread_list + i));
#endif
				pthread_join(*(thread_list + i), NULL);
			}
			break;
		}
	}

	// todo:free task
	tqueue_distoryQueue(tpoll->task_queue);
	pthread_mutex_destroy(&(tpoll->tpoll_mutex));
	pthread_mutex_destroy(&(tpoll->tpoll_read_mutex));
	pthread_cond_destroy(&(tpoll->task_not_empty));
	pthread_cond_destroy(&(tpoll->task_not_full));
	free(tpoll);

#if DEBUG
	printf("pthread_poll clearned.\n");
#endif 

	return 0;
}


