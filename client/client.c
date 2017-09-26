/*************************************************************************
    > File Name: tpool_server.h
    > Author: huguijian
    > Mail: 292438151@qq.com
 ************************************************************************/

#include<stdio.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <time.h>
#include "client.h"


#define SERVER_ADDR_HOST					"127.0.0.1"
static int port  = 8891;

static pthread_t accep_thread_t;
static int connect_total = 0;

typedef int BOOL;
#ifndef FALSE
#define	FALSE						(0)
#endif
#ifndef	TRUE
#define	TRUE						(!FALSE)
#endif

int client_server_data_fun(int port,int index)
{
	int return_value = 0;
	int socket_fd, err, num, loc;
	struct sockaddr_in server_addr;
	int recv_len;
	char recv_buffer[2048] = {0};
    char send_buffer[1024] = {0};
    memset(send_buffer,0,sizeof(send_buffer));
    sprintf(send_buffer,"hello epoll server index:%d\n",index);

    //create socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR_HOST);
	// connect
	err = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(err == 0)
	{

		// send
        num = write(socket_fd, (char *)send_buffer, strlen(send_buffer));
		if (num <= 0) {
			printf("send error\n");
			return -1;
		}
	
		// recv
		num = recv(socket_fd, recv_buffer, 2048, 0);
		if (num > 0) // success
		{
			printf("connect_total = %d\n, rcv buffer = %s\n", connect_total, recv_buffer);
			connect_total++;
			return_value = 2;
            sleep(10);
            write(socket_fd, (char *)send_buffer, strlen(send_buffer));
            printf("connect_total = %d\n, rcv buffer = %s\n", connect_total, recv_buffer);
            connect_total++;
            return_value = 2;
		}
		printf("out\n");
	}
	else
	{
		printf("connect error\n");
		return_value = 1;
	}

	return return_value;
}

static void *accept_thread(void *arg)
{

	int loop_index , index;
    for (loop_index = 0; loop_index < 1; loop_index++)
	{
        printf("loop index is:%d\n",loop_index);
        client_server_data_fun(port,loop_index);

	}
	return NULL;
}

static int create_accept_task(void)
{
	return pthread_create(&accep_thread_t, NULL, accept_thread, NULL);
}

int main()
{
	int index = 0, temp;
	int port_index = 0;
	create_accept_task();
    while(1){

    }
}

