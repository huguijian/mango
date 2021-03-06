/*************************************************************************
    > File Name: tpool_server.h
    > Author: huguijian
    > Mail: 292438151@qq.com
    > Created Time: 2017年07月07日 星期五 12时14分54秒
 ************************************************************************/
#include "mango_global.h"
#include "mango_log.h"


#define TIME_STAMP_BUF_LEN 128

#define mango_mkdir(log_file_path)\
    do{\
        char cmd[100];\
        sprintf(cmd,"mkdir -m 777 -p %s",log_file_path);\
        system(cmd);\
    }while(0);



static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static FILE *log_file_handle = NULL;
static char log_file_name[128];
static char cur_ts_str[TIME_STAMP_BUF_LEN];
static char last_log_str[LOG_STR_BUF_LEN];

void set_log_file_path(char *log_path)
{

    mango_mkdir(log_path);
    struct tm *time_stamp;
    time_t today;
    today = time(NULL);
    time_stamp = localtime(&today);
    strftime(log_file_name,sizeof(log_file_name),"/data/mango_log/%Y%m%d.txt",time_stamp);

}

static int log_create_current_time_stamp(void)
{
    struct tm *time_stamp;
    time_t cur_time;

    cur_time = time(NULL);
    time_stamp = localtime(&cur_time);

    snprintf(cur_ts_str,TIME_STAMP_BUF_LEN,"%02d/%02d/%02d %02d:%02d:%02d",time_stamp->tm_mday,time_stamp->tm_mon+1,time_stamp->tm_year % 100,time_stamp->tm_hour,time_stamp->tm_min,time_stamp->tm_sec);
    return 0;
}

int log_init()
{



    log_file_handle = fopen(log_file_name,"w");
    if(log_file_handle==NULL)
    {

       printf("[%s %s %d] Can't create log file.\n",__FILE__,__FUNCTION__,__LINE__);
       return -1;

    }

    memset(last_log_str,0,LOG_STR_BUF_LEN);
    log_printf(LOG_LEVEL_INDISPENSABLE,"LOG START\n");
    return 0;
}

int log_printf(int level,const char *str)
{
    char log_str[LOG_STR_BUF_LEN];
    memset(log_str,0,sizeof(log_str));
    int need_log_str = 0;
    log_create_current_time_stamp();
    pthread_mutex_lock(&log_mutex);

    if(strcmp(str, last_log_str)) {
        switch(level)
        {
            case LOG_LEVEL_INFO:
                need_log_str = 1;
                snprintf(log_str,LOG_STR_BUF_LEN,"[INFO: %s] %s",cur_ts_str,str);
                break;

            case LOG_LEVEL_WARNING:
                need_log_str = 1;
                snprintf(log_str,LOG_STR_BUF_LEN,"[WARNING: %s] %s",cur_ts_str,str);
                break;

            case LOG_LEVEL_ERROR:
                need_log_str = 1;
                snprintf(log_str,LOG_STR_BUF_LEN,"[ERROR: %s] %s",cur_ts_str,str);
                break;

            case LOG_LEVEL_FATAL:
                need_log_str = 1;
                snprintf(log_str,LOG_STR_BUF_LEN,"[FATAL: %s] %s",cur_ts_str,str);
                break;

            case LOG_LEVEL_INDISPENSABLE:
                need_log_str = 1;
                snprintf(log_str,LOG_STR_BUF_LEN,"[%s] %s",cur_ts_str,str);
                break;
        }

        if(need_log_str)
        {
            fprintf(log_file_handle,"%s",log_str);
            memset(last_log_str,0,LOG_STR_BUF_LEN);
            strcpy(last_log_str,str);
        }
        fflush(log_file_handle);
    }
    pthread_mutex_unlock(&log_mutex);
    return 0;
}

int log_close(void)
{
    if(log_file_handle != NULL)
    {
        log_printf(LOG_LEVEL_INDISPENSABLE,"LOG END\n");
        fclose(log_file_handle);
    }
    return 0;
}


