/*************************************************************************
    > File Name: tpool_server.h
    > Author: huguijian
    > Mail: 292438151@qq.com
    > Created Time: 2017年07月07日 星期五 12时14分54秒
 ************************************************************************/

#ifndef _MANGO_GLOBAL_H
#define _MANGO_GLOBAL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/tcp.h>

#ifndef KEEP_ALIVE_TIME
#define KEEP_ALIVE_TIME 15
#endif

typedef int BOOL;
#ifndef FALSE
#define	FALSE						(0)
#endif
#ifndef	TRUE
#define	TRUE						(!FALSE)
#endif

#endif
//