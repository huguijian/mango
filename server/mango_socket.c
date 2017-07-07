/*************************************************************************
	> File Name: mango_socket.c
	> Author: 
	> Mail: 
	> Created Time: 2017年07月07日 星期五 16时28分20秒
 ************************************************************************/
#include "mango_global.h"
#include "mango_socket.h"

ssize_t socket_send(int fd, void *buf, size_t count)
{
        int left = count ;
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

void socket_close(int fd)
{
    shutdown(fd,SHUT_RDWR);
}

int socket_set_non_block(int fd)
{
    int opts = -1;
    opts = fcntl(fd,F_GETFL);
    if(opts < 0){
        printf("fcntl(fd=%d,GETFL) error.\n",fd);
        return -1;
    }
    opts = opts | O_NONBLOCK;
    if(fcntl(fd,F_SETFL,opts)<0) {
        printf("fcntl(fd=%d,GETFL).\n error",fd);
        return -1;
    }
    return 0;
}
