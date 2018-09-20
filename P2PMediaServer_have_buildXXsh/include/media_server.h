/*********************************************************************************
  *FileName: media_server.h
  *Create Date: 2018/09/18
  *Description: 设备软件端Media Server程序业务逻辑处理的主头文件
  *Others:  
  *History:  
**********************************************************************************/
#ifndef MEDIA_SERVER_H
#define MEDIA_SERVER_H


//ntp对时函数，位于静态库当中（libntpclient.a）
//extern "C" int ntp_sync_time(int argc, char *argv[]);
extern "C" void ntp_sync_time(void);


#endif








