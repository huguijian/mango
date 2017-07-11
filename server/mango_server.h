/*************************************************************************
	> File Name: tpool_server.h
	> Author: 
	> Mail: 
	> Created Time: 2017年07月07日 星期五 12时14分54秒
 ************************************************************************/
#ifndef _MANGO_SERVER_H
#define _MANGO_SERVER_H


#define BUFFLEN 1024
#define SERVER_PORT 8891
#define BACKLOG 5
#define CLIENTNUM 1024

#define EPOLLEVENTS 20
#define FDSIZE 1000
#define MAXSIZE 1024
#define READ_DATA_MODEL 2

int connect_host[CLIENTNUM];
int connect_number = 0;
static int epoll_fd = -1;



struct dataPacket{
    unsigned int msgLen;
    char data[1024];
};

typedef struct _task_func_paramter{
    int fd;
    char *recv_buffer;
}task_func_paramter;

typedef struct _task_queue{

    void *(*task_func)(task_func_paramter *arg);
    task_func_paramter *func_paramter;
    struct _task_queue *prev;
    struct _task_queue *next;
}task_queue;

typedef struct _tpool{
    pthread_mutex_t queue_lock;
    pthread_cond_t  queue_ready;

    pthread_t *threads;
    int        thread_num;
    task_queue *first_task;
    int task_queue_num;

}tpool;

typedef struct _tpool_thread_paramter{
    tpool *tpool_t;
    int   thread_index;
}tpool_thread_paramter;

void socket_close(int fd);

#endif
