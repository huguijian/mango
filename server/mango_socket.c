/*************************************************************************
	> File Name: mango_socket.c
    > Author: huguijian
    > Mail: 292438151@qq.com
	> Created Time: 2017年07月07日 星期五 16时28分20秒
 ************************************************************************/
#include "mango_global.h"
#include "mango_socket.h"
#include "mango_log.h"
#include "thirdparty/http-parser/http_parser.h"
typedef struct {
  int sock;
  char *buffer;
  int buf_len;
 } http_data_info;

ssize_t socket_send(int fd, void *buf, size_t count)
{
        int left = count;
        char * ptr = (char *)buf;
        while(left >0)
        {
                int writeBytes = send(fd,ptr,left,0);
                if(writeBytes<0)
                {
                        if(errno == EINTR)
                                continue;
                        return -1;
                }
                else if(writeBytes == 0)
                        continue;
                left -= writeBytes;
                ptr  += writeBytes;
        }
        return left;
}

ssize_t socket_recv(int fd,void *buf,size_t count)
{
    int left = count ; //剩下的字节
    char *ptr = (char*)buf ;
    while(left>0)
    {
            int readBytes = recv(fd,ptr,left,0);

            if(readBytes< 0)//read函数小于0有两种情况：1中断 2出错
            {
                    if(errno == EINTR)//读被中断
                    {
                            continue;
                    }

                    return -1;
            }
            if(readBytes == 0)//读到了EOF
            {

                return count-left;
            }
            left -= readBytes;
            ptr  += readBytes;
    }

    return left;
}

ssize_t socket_recv_all(int fd,void *buf,size_t count)
{
    char *ptr = (char*)buf ;
    int readBytes = 0;
    do{
        readBytes = recv(fd,ptr,count,0);
        if(readBytes< 0)//read函数小于0有两种情况：1中断 2出错
        {
                if(errno == EINTR)//读被中断
                {
                        continue;
                }

                return -1;
        }
        ptr+=readBytes;
    }while(readBytes>0);

    return readBytes;
}

ssize_t socket_recv_peek(int fd,char *buf,size_t len)
{
    while(1)
    {
        int ret = recv(fd,buf,len,MSG_PEEK);
        if(ret==-1 && errno==EINTR)
            continue;
        return ret;
    }
}

ssize_t socket_recv_by_eof(int fd, void *buf,size_t max_size)
{
    char log_str_buf[LOG_STR_BUF_LEN];
    int ret;
    int nRead = 0;
    int left = max_size;
    char *pbuf = (char*) buf;
    int count  = 0;

    while(1)
    {
        ret = socket_recv_peek(fd,pbuf,left);

        if(ret<=0)
        {
            return ret;
        }

        nRead = ret;
        for(int i=0;i<nRead;++i)
        {
            if(pbuf[i] == '\n')
            {

                ret = socket_recv(fd,pbuf,i+1);

                if(ret != 0){
                    snprintf(log_str_buf,LOG_STR_BUF_LEN,"Recv data len error .data len is:%d\n",ret);
                    LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);
                }

                return ret;
            }
        }

        ret = socket_recv(fd,pbuf,nRead);
        pbuf += nRead;
        count +=nRead;
    }

    return -1;
}

void socket_close(int fd)
{
    shutdown(fd,SHUT_RDWR);
    close(fd);
}

int socket_set_non_block(int fd)
{
    int opts = -1;
    char log_str_buf[LOG_STR_BUF_LEN];
    opts = fcntl(fd,F_GETFL);
    if(opts < 0){

        snprintf(log_str_buf,LOG_STR_BUF_LEN,"Socket set fcntl(fd=%d,F_GETFL) error.\n",fd);
        LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);
        return -1;
    }
    opts = opts | O_NONBLOCK;
    if(fcntl(fd,F_SETFL,opts)<0) {
        snprintf(log_str_buf,LOG_STR_BUF_LEN,"Socket set fcntl(fd=%d,F_SETFL).\n error",fd);
        LOG_INFO(LOG_LEVEL_ERROR,log_str_buf);
        return -1;
    }
    return 0;
}

int on_message_begin(http_parser *parser)
{
    printf("\n***MESSAGE BEGIN***\n");
    return 0;
}

int on_header_field(http_parser *parser,const char *at,size_t length)
{
    printf("Head field:%.*s\n",(int)length,at);
    return 0;
}

int on_header_value(http_parser *parser,const char *at,size_t length)
{
    printf("Header value:%.*s\n",(int)length,at);
    return 0;
}

int on_url(http_parser *parser, const char *at, size_t length)
{

  printf("Url: %.*s\n", (int)length, at);
  return 0;
}

int on_status(http_parser *parser,const char *at,size_t length)
{
    printf("Status value:%.*s\n",(int)length,at);
    return 0;
}

int on_body(http_parser *parser,const char *at,size_t length)
{
    printf("body value:%.*s\n",(int)length,at);
    return 0;
}

int on_headers_complete(http_parser *parser)
{
    printf("\n***HEADERS COMPLETE***\n");
    return 0;
}

int on_message_complete(http_parser *parser)
{
    printf("\n***MESSAGE COMPLETE***\n");
    return 0;
}

//http parser callback function
static http_parser_settings settings =
{
    .on_message_begin = on_message_begin,
    .on_header_field = on_header_field,
    .on_header_value = on_header_value,
    .on_url = on_url,
    .on_status = 0,
    .on_body = on_body,
    .on_headers_complete = on_headers_complete,
    .on_message_complete = on_message_complete
};

void http_parser_thread(int sock,char *requestBuf) {

    http_parser *parser = malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);

    /* execute parser */
    http_parser_execute(parser, &settings, requestBuf, strlen(requestBuf));
    printf("comm on \n");
}

