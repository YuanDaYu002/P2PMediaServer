/*********************************************************************************
  *FileName: media_server_plog.h
  *Create Date: 2018/09/19
  *Description: 负责log打印的头文件 
  *Others:  
  *History:  
**********************************************************************************/

#ifndef MEDIA_SERVER_PLOG_H
#define MEDIA_SERVER_PLOG_H

#include <syslog.h>
#include <stdio.h>


const char* get_src_name(const char* fpath);
//-DDEBUG 放在makefile里边定义
#if defined(DEBUG)
/*
#define plog(fmt, ...)  	printf("<%s>[%u]%s  #" fmt, get_src_name(__FILE__),__LINE__,__FUNCTION__,## __VA_ARGS__)
#define plogerr(fmt,...) 	printf("<Error!> <%s>[%u]%s  #" fmt, get_src_name(__FILE__),__LINE__,__FUNCTION__,## __VA_ARGS__)
#define plogfn()  		printf("<%s>[%u]%s :[%lu]\n", get_src_name(__FILE__),__LINE__, __FUNCTION__, pthread_self())
*/
#define plog(fmt, ...)  	printf("<%s>[%u]  :" fmt, get_src_name(__FILE__),__LINE__,## __VA_ARGS__)
#define plogerr(fmt,...) 	printf("<Error!> <%s>[%u]  :" fmt, get_src_name(__FILE__),__LINE__,## __VA_ARGS__)
#define plogfn()  		printf("<%s>[%u]%s :[tid:%lu]\n", get_src_name(__FILE__),__LINE__, __FUNCTION__, pthread_self())

#else
#define plog(fmt, ...)
#define plogerr(fmt,...)
#define plogfn()
#endif


#define plog_str(s) plog( # s " -> %s\n", s)
#define plog_int(v) plog( # v" -> %d\n", v)

#endif /* end of include guard: LOG_H */







