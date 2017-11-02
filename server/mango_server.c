/*************************************************************************
    > File Name: tpool_server.h
    > Author: huguijian
    > Mail: 292438151@qq.com
    > Created Time: 2017年07月07日 星期五 12时14分54秒
 ************************************************************************/
#include "mango_global.h"
#include "mango_connect.h"
#include "mango_server.h"
#include "mango_socket.h"
#include "thirdparty/http-parser/http_parser.h"
#include "mango_log.h"

void *thread_handle(tpool_thread_paramter *arg)
{

    while (1)
    {
        pthread_mutex_lock (&(arg->tpool_t->queue_lock));

        while (arg->tpool_t->task_queue_num == 0)
        {
            pthread_cond_wait (&(arg->tpool_t->queue_ready), &(arg->tpool_t->queue_lock));
        }

        /*等待队列长度减去1，并取出链表中的头元素*/
        arg->tpool_t->task_queue_num--;
        task_queue *worker = arg->tpool_t->first_task;
        arg->tpool_t->first_task = worker->next;
        pthread_mutex_unlock (&(arg->tpool_t->queue_lock));
        /*调用回调函数，执行任务*/

        (*(worker->task_func)) (worker->func_paramter);
        free (worker);
        worker = NULL;
    }
}

int tpool_add_task(tpool *tpool_t,void*(*task_func)(task_func_paramter *arg),task_func_paramter *arg)
{
    task_queue *task_queue_t    = (task_queue*) malloc(sizeof(task_queue));

    task_queue_t->task_func     = task_func;
    task_queue_t->func_paramter = arg;
    task_queue_t->next = NULL;

    pthread_mutex_lock (&(tpool_t->queue_lock));
    /*将任务加入到等待队列中*/
    task_queue *curr_task = tpool_t->first_task;
    if (curr_task != NULL)
    {
        while (curr_task->next != NULL)
          curr_task = curr_task->next;
        curr_task->next = task_queue_t;
    }
    else
    {
        tpool_t->first_task = task_queue_t;
    }

    tpool_t->task_queue_num++;

    pthread_mutex_unlock(&(tpool_t->queue_lock));
    pthread_cond_signal(&(tpool_t->queue_ready));

    return 0;
}

static void *tpool_task_function(task_func_paramter *arg)
{

  printf("current connected fd=%d\n",arg->fd);
  //char log_str_buf[LOG_STR_BUF_LEN];
  //snprintf(log_str_buf,LOG_STR_BUF_LEN,"current recv data :%s\n",arg->recv_buffer);
  //LOG_INFO(LOG_LEVEL_INFO,log_str_buf);
  printf("current recv data :%s\n",arg->recv_buffer);
  socket_send(arg->fd,arg->recv_buffer,strlen(arg->recv_buffer));

  if(arg->fd != -1)
  {
      //int connect_index = get_connect_index_by_fd(arg->fd);
      //set_free_connect_by_index(connect_index);
      //connect_total(FALSE,1);
      //socket_close(arg->fd);

  }

  return 0;
}

tpool *init_tpool(int tpool_num)
{
    int i=0;
    char log_str_buf[LOG_STR_BUF_LEN];
    tpool *tpool_t = NULL;
    if(tpool_num<=0) {

        LOG_INFO(LOG_LEVEL_WARNING,"Thread pool num must be greater than 0\n");
    }

    tpool_t = (tpool*)malloc(sizeof(tpool));

    pthread_mutex_init(&(tpool_t->queue_lock),NULL);
    pthread_cond_init(&(tpool_t->queue_ready),NULL);

    tpool_t->task_queue_num = 0;
    tpool_t->threads    = (pthread_t*)malloc(tpool_num*sizeof(pthread_t));
    tpool_t->thread_num =  tpool_num;
    tpool_thread_paramter *tpool_thread_paramter_t = (tpool_thread_paramter*)malloc(sizeof(tpool_thread_paramter));
    for(i=0;i<tpool_num;i++) {
        tpool_thread_paramter_t->tpool_t = tpool_t;
        tpool_thread_paramter_t->thread_index = i;
        if(pthread_create(&(tpool_t->threads[i]),NULL,(void*)thread_handle,tpool_thread_paramter_t)!=0) {
            LOG_INFO(LOG_LEVEL_ERROR,"Create thread fail!\n");
        }
    }

    return tpool_t;

}


static void *accept_thread(void *arg)
{
    int connect_fd = -1;
    int val = 1;
    int err = 0;
    int bufsize = 32*1024;
    int connect_index = 0;

    char log_str_buf[LOG_STR_BUF_LEN];

    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    struct epoll_event ev;
    static int listen_fd = -1;
    listen_fd = socket(AF_INET,SOCK_STREAM,0);

    if(-1 == listen_fd) {

        LOG_INFO(LOG_LEVEL_ERROR,"Create socket error\n");
        return NULL;
    }

    err = setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
    if(err != 0) {

        LOG_INFO(LOG_LEVEL_ERROR,"Set socket options SO_REUSEADDR error.\n");
        return NULL;
    }

    err = setsockopt(listen_fd,SOL_SOCKET,SO_RCVBUF,(char*)(&bufsize),sizeof(int));
    if(err != 0) {

        LOG_INFO(LOG_LEVEL_ERROR,"Set socket options SO_RCVBUF error.\n");
        return NULL;
    }

    val = 2;
    err = setsockopt(listen_fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &val, sizeof(val));
    if(err != 0) {

        LOG_INFO(LOG_LEVEL_ERROR,"Set socket options TCP_DEFER_ACCEPT error.\n");
        return NULL;
    }


    bzero(&serveraddr,sizeof(serveraddr));
    //memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);

    err = bind(listen_fd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
    if(err == -1) {

        LOG_INFO(LOG_LEVEL_ERROR,"Socket bind error.\n");
        return NULL;
    }

    listen(listen_fd,BACKLOG);
    socklen_t len = sizeof(clientaddr);
    while(1)
    {
        if((connect_fd = accept(listen_fd,(struct sockaddr*)&clientaddr,&len))<0){
            LOG_INFO(LOG_LEVEL_ERROR,"Accept error.\n");
        }


        connect_index = get_free_connect_index();


        if(connect_index==-1) {

            LOG_INFO(LOG_LEVEL_ERROR,"Not found free connect.\n");

            if(connect_fd != -1) {
                socket_close(connect_fd);
                connect_fd = -1;
            }

            continue;
        }

        if(socket_set_non_block(connect_fd)<0){
            snprintf(log_str_buf,LOG_STR_BUF_LEN,"Set socket noblock error by socket connect fd=%d\n",connect_fd);
            LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);

            if(connect_fd != -1){
                socket_close(connect_fd);
                connect_fd = -1;
            }
            continue;
        }

        err = setsockopt(connect_fd,SOL_SOCKET,SO_RCVBUF,(char *)(&bufsize),sizeof(int));
        if(err != 0)
        {
            snprintf(log_str_buf,LOG_STR_BUF_LEN,"Set socket options SO_RCVBUF error by socket connect fd=%d\n",connect_fd);
            LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);
            if(connect_fd != -1){
                socket_close(connect_fd);
                connect_fd = -1;
            }
            continue;
        }

        add_connect_to_pool_connect(connect_index,connect_fd,inet_ntoa(clientaddr.sin_addr));

        ev.data.fd = connect_fd;
        ev.events  = EPOLLIN | EPOLLET;
        if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,connect_fd,&ev) == -1){
            LOG_INFO(LOG_LEVEL_ERROR,"Epoll EPOLL_CTL_ADD error.\n");

            if(connect_fd != -1){
                socket_close(connect_fd);
                connect_fd = -1;
            }
        }

        connect_total(TRUE,1);

        snprintf(log_str_buf,LOG_STR_BUF_LEN,"Epoll event[%d] Current connected total num %d from ip:%s fd:%d.\n",connect_index,get_connect_count(),inet_ntoa(clientaddr.sin_addr),connect_fd);
        LOG_INFO(LOG_LEVEL_INFO,log_str_buf);
    }

    if(listen_fd != -1){
        //socket_close(listen_fd);

        listen_fd = -1;
    }

    return NULL;
}

static int create_accept_task(void)
{
   static pthread_t accep_thread_t;
   return pthread_create(&accep_thread_t,NULL,accept_thread,NULL);
}

int main(int argc,char *argv[])
{
    set_log_file_path("/data/mango_log/");
    log_init();
    char log_str_buf[LOG_STR_BUF_LEN];
    int epoll_event_number = 0;
    int index = 0;
    int connected_fd = -1;
    struct epoll_event ev,events[MAX_FDS];
    struct dataPacket readPacket,writePacket;
    init_pool_connect();
    create_accept_task();
    thread_heartbeat();

    epoll_fd = epoll_create(MAX_FDS);
    tpool *tpool_t = NULL;
    tpool_t = init_tpool(5);



    while(1)
    {
        epoll_event_number = epoll_wait(epoll_fd,events,MAX_FDS,2000);

        for(index=0;index<epoll_event_number;index++) {
            /*
            if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,events[index].data.fd,&ev) == -1)
            {

                LOG_INFO(LOG_LEVEL_ERROR,"Epoll ctl delete error.\n");
                events[index].data.fd = -1;
            }
            **/
            connected_fd = events[index].data.fd;
            if(events[index].events & EPOLLIN){

                int connect_index = -1;
                if(connected_fd < 0){
                    connect_total(FALSE,1);
                    snprintf(log_str_buf,LOG_STR_BUF_LEN,"Event[%d] read invalid handle.\n",index);
                    LOG_INFO(LOG_LEVEL_WARNING,log_str_buf);
                    continue;
                }

                connect_index = get_connect_index_by_fd(connected_fd);
                update_connect_time(connect_index);
                if(connect_index < 0)
                {
                    connect_total(FALSE,1);
                    set_free_connect_by_index(connect_index);
                    if(connected_fd != -1)
                    {
                        socket_close(connected_fd);
                        connected_fd = -1;
                    }
                    continue;
                }

                //recv data
                char request_buf[102400];
                char data_buf[1024];
                int msg_len = 0;
                int readBytes = 0;
                int dataBytes;
                switch (READ_DATA_MODEL) {
                case 1:

                    memset(&readPacket,0,sizeof(readPacket));
                    msg_len = socket_recv(connected_fd,&readPacket.msgLen,4);

                    dataBytes = ntohl(readPacket.msgLen); //字节序的转换
                    readBytes = socket_recv(connected_fd,&readPacket.data,dataBytes); //读取出后续的数据
                    break;
                case 2:

                    memset(data_buf,0,sizeof(data_buf));
                    readBytes = socket_recv_by_eof(connected_fd,&data_buf,1024);

                    break;
                case 3:
                    //memset(request_buf,0,sizeof(request_buf));
                    readBytes = socket_recv_all(connected_fd,request_buf,10);
                    printf("request buf is :%s\n",request_buf);
                    readBytes = 0;
                    break;
                default:
                    break;
                }


                if(readBytes==0)
                {

                    task_func_paramter *task_func_paramter_t = (task_func_paramter *)malloc(sizeof(task_func_paramter));
                    memset(task_func_paramter_t,0,sizeof(task_func_paramter));
                    task_func_paramter_t->fd = connected_fd;
                    switch(READ_DATA_MODEL){
                        case 1:
                            task_func_paramter_t->recv_buffer = (char*)malloc(sizeof(readPacket.data));
                            strcpy(task_func_paramter_t->recv_buffer,readPacket.data);
                            break;

                        case 2:
                            task_func_paramter_t->recv_buffer = data_buf;
                            break;
                        case 3:

                             http_parser_thread(connected_fd,request_buf);

                             task_func_paramter_t->recv_buffer = "haha\n";
                            break;
                        default:
                            break;
                    }


                    tpool_add_task(tpool_t,tpool_task_function,task_func_paramter_t);
                }else{

                    connect_total(FALSE,1);
                    //set_free_connect_by_index(connect_index);
                    if(connected_fd != -1)
                    {
                        socket_close(connected_fd);
                        connected_fd = -1;
                    }

                }


            }else{

                connect_total(FALSE,1);
                if(connected_fd<0)
                {
                    snprintf(log_str_buf,LOG_STR_BUF_LEN,"Connect fd error for connect_fd is %d",connected_fd);
                    LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);
                    continue;
                }

                //set_free_connect_by_index(get_connect_index_by_fd(connected_fd));
                if(connected_fd != -1){
                    //socket_close(connected_fd);
                    connected_fd = -1;
                }

            }
        }
    }
    
    //close(s_s);
    return 0;
    
}
