#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>


#include <unistd.h>

#include "tpoll.h"
#include "tqueue.h"

int addTask(tpoll_t *tpoll);

void *test_routine(void *arg) {
	write(STDOUT_FILENO, (char *)arg, strlen((char *)arg));
	sleep(1);
	return NULL;
}

int main(void) {
	// create tpoll
	tpoll_t *tpoll;
	tqueue_t *tqueue;
	int ret = tpoll_create(&tpoll, 6, 10, &tqueue, 5);
	if (ret != 0) {
		fprintf(stderr, "create tpoll error \n");
		exit(1);
	}
	printf("start \n");
	sleep(5);

	// test add task
	/*
	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	tpoll_addTask(tpoll, test_routine, (void *)"test routine 1");
	tpoll_addTask(tpoll, test_routine, (void *)"test routine 2");
	pthread_mutex_unlock(&(tpoll->tpoll_mutex));



	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	tpoll_addTask(tpoll, test_routine, (void *)"test 2 routine 1");
	tpoll_addTask(tpoll, test_routine, (void *)"test 2 routine 2");	
	tpoll_addTask(tpoll, test_routine, (void *)"test 2 routine 3");
	tpoll_addTask(tpoll, test_routine, (void *)"test 2 routine 4");
	pthread_mutex_unlock(&tpoll->tpoll_mutex);

	sleep(10);
	*/

	// test full task added
	addTask(tpoll);
	sleep(5);
	// 测试创建新的线程
	printf("test add new thread \n");
	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	ret = tpoll_increaseThread(tpoll, 3);
	if (ret <= 0) {
		printf("increase error %d \n", ret);
	}
	printf("tpoll thread: %d \n", tpoll->thread_curr_cnt);
	pthread_mutex_unlock(&(tpoll->tpoll_mutex));

	addTask(tpoll);
	sleep(5);

	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	ret = tpoll_decreaseThread(tpoll, 3);
	if (ret < 0) {
		printf("decrease thread error : %d \n", ret);
	}
	printf("tpoll thread: %d \n", tpoll->thread_curr_cnt);
	pthread_mutex_unlock(&(tpoll->tpoll_mutex));

	addTask(tpoll);
	sleep(5);
	
	printf("start distory \n");	
	pthread_t manager_tid = (tpoll->manager_thread);
	pthread_mutex_lock(&(tpoll->tpoll_mutex));
	tpoll_distoryTpoll(tpoll);
	pthread_mutex_unlock(&(tpoll->tpoll_mutex));

	pthread_join(manager_tid, NULL);

	sleep(100);

	return 0;
}

int addTask(tpoll_t *tpoll) {
	int count = 0;
	while (1) {
		if (count >= 10) {
			break;
		}
		pthread_mutex_lock(&(tpoll->tpoll_mutex));
		while (tqueue_full(tpoll->task_queue) && tpoll->shutdown == 0) {
			pthread_cond_wait(&(tpoll->task_not_full), &(tpoll->tpoll_mutex));
		}
		tpoll_addTask(tpoll, test_routine, (void *)"test 3 routine while \n");
		pthread_mutex_unlock(&(tpoll -> tpoll_mutex));
		count++;
	}
	return 0;
}
