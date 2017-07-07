/*************************************************************************
	> File Name: pool_connect.h
	> Author: 
	> Mail: 
	> Created Time: 2017年07月07日 星期五 12时10分19秒
 ************************************************************************/
#ifndef _MANGO_CONNECT_H
#define _MANGO_CONNECT_H
#define MAX_FDS 1024
#define MAX_CONNECT_NUM MAX_FDS
#define IP_ADDR_LENGTH 20

typedef struct _pool_connect{
    int connect_fd;
    int socket_status;
    time_t now;
    char client_ip_addr[IP_ADDR_LENGTH];
    pthread_mutex_t mutex;
}pool_connect;

static pool_connect pool_connect_client[MAX_CONNECT_NUM];

int get_connect_count();
int connect_total(BOOL is_true,int value);
void init_pool_connect(void);
void add_connect_to_pool_connect(int iConnect,int fd,char *clientIp);
pool_connect get_connect_info_by_index(int index);
int get_free_connect_index(void);
void set_free_connect_by_index(int index);
int get_connect_index_by_fd(int connected_fd);
#endif
