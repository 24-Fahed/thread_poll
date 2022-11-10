tpoll_test_main.o: tpoll_test_main.c tpoll.c tqueue.c tpoll.h tqueue.h
	gcc -g tpoll_test_main.c tpoll.c tqueue.c -o tpoll_test_main.o -I./ -pthread
