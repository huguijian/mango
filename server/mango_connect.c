/*************************************************************************
    > File Name: tpool_server.h
    > Author: huguijian
    > Mail: 292438151@qq.com
    > Created Time: 2017年07月07日 星期五 12时14分54秒
 ************************************************************************/
#include "mango_global.h"
#include "mango_connect.h"
#include "mango_log.h"
#include "mango_socket.h"
static int current_connected_total = 0;
static pthread_mutex_t connect_total_mutex = PTHREAD_MUTEX_INITIALIZER;//静态锁与动态锁多次被定义问题
int get_connect_count();
static void lock_event_state(int iConnect,int iLock)
{
    char log_str_buf[LOG_STR_BUF_LEN];
    int iRet;
    if(iLock){
        iRet = pthread_mutex_lock(&pool_connect_client[iConnect].mutex);
    }else{
        iRet = pthread_mutex_unlock(&pool_connect_client[iConnect].mutex);
    }

    if(iRet!=0){
        snprintf(log_str_buf,LOG_STR_BUF_LEN,"iConnect[%d] mutex lock[%d]\n",iConnect,iLock);
        LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);
    }
}

void init_pool_connect()
{
    char log_str_buf[LOG_STR_BUF_LEN];
    int iIndex;
    int iRet;
    memset((char*)pool_connect_client,0,sizeof(pool_connect_client));
    for(iIndex = 0;iIndex<MAX_CONNECT_NUM;iIndex++)
    {
        pool_connect_client[iIndex].connect_fd = -1;
        pool_connect_client[iIndex].socket_status = 0;
        iRet = pthread_mutex_init(&pool_connect_client[iIndex].mutex,NULL);
        if(iRet != 0){
            LOG_INFO(LOG_LEVEL_ERROR,"Pthread_mutex_init error.\n");
        }
    }
}

void add_connect_to_pool_connect(int iConnect,int fd,char *clientIp)
{
    time_t now;
    time(&now);

    lock_event_state(iConnect,1);
    pool_connect_client[iConnect].connect_fd = fd;
    pool_connect_client[iConnect].socket_status = 1;
    pool_connect_client[iConnect].now = now;
    memset(pool_connect_client[iConnect].client_ip_addr,0,IP_ADDR_LENGTH);
    memcpy(pool_connect_client[iConnect].client_ip_addr,clientIp,IP_ADDR_LENGTH);
    lock_event_state(iConnect,0);

}

int get_free_connect_index(void)
{
    int iIndex;
    for(iIndex=0;iIndex<MAX_CONNECT_NUM;iIndex++){
        if(pool_connect_client[iIndex].connect_fd==-1){
            return iIndex;
        }
    }
    return (-1);
}

void set_free_connect_by_index(int index)
{
    if(index>=0 && index<MAX_CONNECT_NUM){
        lock_event_state(index,1);
        pool_connect_client[index].connect_fd = -1;
        pool_connect_client[index].socket_status = 0;
        lock_event_state(index,0);
    }
}

int get_connect_index_by_fd(int connected_fd)
{
    int iIndex;
    for(iIndex=0;iIndex<MAX_CONNECT_NUM;iIndex++){
        if(pool_connect_client[iIndex].connect_fd == connected_fd)
        {
            return iIndex;
        }
    }
    return (-1);
}

pool_connect get_connect_info_by_index(int index)
{
    if(index>=0 && index<MAX_CONNECT_NUM){
        return pool_connect_client[index];
    }
}

int get_connect_count()
{

    return current_connected_total;
}

int connect_total(BOOL is_true,int value)
{
    pthread_mutex_lock(&connect_total_mutex);
    if(is_true){
        current_connected_total = current_connected_total+value;
    }else{
        current_connected_total = current_connected_total-value;
    }
    pthread_mutex_unlock(&connect_total_mutex);
}

int update_connect_time(int iConnect)
{
    time_t now;
    time(&now);

    lock_event_state(iConnect,1);
    pool_connect_client[iConnect].now = now;
    lock_event_state(iConnect,0);
    return 0;
}

static void *heartbeat_packet_check()
{
    int index = 0;
    time_t keep_alive_time = KEEP_ALIVE_TIME;
    time_t time_difference;
    time_t curr_time;
    while(1)
    {

        connect_go_check:for(index;index<MAX_CONNECT_NUM;index++){

           time(&curr_time);

           if(pool_connect_client[index].connect_fd!=-1)
           {

               time_difference = curr_time-(pool_connect_client[index].now);

               if(time_difference>keep_alive_time)
               {
                   socket_close(pool_connect_client[index].connect_fd);
                   set_free_connect_by_index(index);
                   connect_total(FALSE,1);

               }
           }

           if((index+1)==MAX_CONNECT_NUM){
               sleep(1);
               index = 0;
               goto connect_go_check;
           }

        }



    }
}

int thread_heartbeat()
{
    static pthread_t heartbeat_thread_t;

    return pthread_create(&heartbeat_thread_t,NULL,heartbeat_packet_check,NULL);
}



