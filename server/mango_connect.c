#include "mango_global.h"
#include "mango_connect.h"
static int current_connected_total = 0;
static pthread_mutex_t connect_total_mutex = PTHREAD_MUTEX_INITIALIZER;//静态锁与动态锁多次被定义问题
int get_connect_count();
static void lock_event_state(int iConnect,int iLock)
{
    int iRet;
    if(iLock){
        iRet = pthread_mutex_lock(&pool_connect_client[iConnect].mutex);
    }else{
        iRet = pthread_mutex_unlock(&pool_connect_client[iConnect].mutex);
    }

    if(iRet!=0){
        printf("iConnect[%d] mutex lock[%d]\n",iConnect,iLock);
    }
}

void init_pool_connect()
{
    int iIndex;
    int iRet;
    memset((char*)pool_connect_client,0,sizeof(pool_connect_client));
    for(iIndex = 0;iIndex<MAX_CONNECT_NUM;iIndex++)
    {
        pool_connect_client[iIndex].connect_fd = -1;
        pool_connect_client[iIndex].socket_status = 0;
        iRet = pthread_mutex_init(&pool_connect_client[iIndex].mutex,NULL);
        if(iRet != 0){
            printf("pthread_mutex_init error.\n");
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



