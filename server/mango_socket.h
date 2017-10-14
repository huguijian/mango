/*************************************************************************
	> File Name: mango_socket.h
    > Author: huguijian
    > Mail: 292438151@qq.com
	> Created Time: 2017年07月07日 星期五 16时28分32秒
 ************************************************************************/
#ifndef _MANGO_SOCKET_H
#define _MANGO_SOCKET_H
#include <sys/types.h>

#define DATA_EOF "\n"
ssize_t socket_send(int fd, void *buf, size_t count);
ssize_t socket_recv(int fd,void *buf,size_t count);
ssize_t socket_recv_by_eof(int fd, void *buf,size_t max_size);
void socket_close(int fd);
int socket_set_non_block(int fd);
void http_parser_thread(int sock);
#endif
