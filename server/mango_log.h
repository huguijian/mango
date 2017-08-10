/*************************************************************************
	> File Name: mango_log.h
	> Author: 
	> Mail: 
	> Created Time: 2017年07月07日 星期五 16时29分20秒
 ************************************************************************/

#ifndef _MANGO_LOG_H
#define _MANGO_LOG_H

#define LOG_LEVEL_INFO			(0x00)
#define LOG_LEVEL_WARNING		(0x01)
#define LOG_LEVEL_ERROR			(0x02)
#define LOG_LEVEL_FATAL			(0x03)
#define LOG_LEVEL_INDISPENSABLE	(0x04)	//To display several indispensable information.

#define LOG_STR_BUF_LEN		256

#define LOG_INFO(a,b)\
{\
    char log_str[LOG_STR_BUF_LEN];\
    snprintf(log_str,LOG_STR_BUF_LEN,"[%s %s %d] %s",__FILE__,__FUNCTION__,__LINE__,b);\
    log_printf(a,log_str);\
}

void set_log_file_name(char *file_name);
int log_init();
int log_printf(int level,const char *str);
int log_close(void);

#endif
