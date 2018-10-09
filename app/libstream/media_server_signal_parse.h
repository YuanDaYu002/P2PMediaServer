/*********************************************************************************
  *FileName: media_server_signal_parse.h
  *Create Date: 2018/09/19
  *Description: 客户端信令解析的头文件，用户事件处理函数 
  *Others:  
  *History:  
**********************************************************************************/
#ifndef MEDIA_SERVER_SIGNAL_PARSE_H
#define MEDIA_SERVER_SIGNAL_PARSE_H

#include "typeport.h"


HLE_S32 med_ser_signal_parse(HLE_S32 SessionID,void*data,HLE_S32 length);

HLE_S32 cmd_Get_Mac(HLE_S32 SessionID);						//获取设备MAC地址
HLE_S32 cmd_read_dev_info(HLE_S32 SessionID);				//读设备信息	
HLE_S32 cmd_set_dev_para(HLE_S32 SessionID);  				//设置设备系统参数	
HLE_S32 cmd_read_dev_para(HLE_S32 SessionID); 				//读取设备系统参数	
HLE_S32 cmd_alarm_update(HLE_S32 SessionID); 				//报警通知
HLE_S32 cmd_open_living(HLE_S32 SessionID); 					//打开实时流传输
HLE_S32 cmd_close_living(HLE_S32 SessionID); 				//关闭实时流传输
HLE_S32 cmd_set_reboot(HLE_S32 SessionID); 					//重启命令
HLE_S32 cmd_set_update(HLE_S32 SessionID); 					//升级命令
HLE_S32 cmd_set_wifi_connect(HLE_S32 SessionID); 			//连接WIFI热点		
HLE_S32 cmd_get_wifi_status(HLE_S32 SessionID); 				//获取wifi连接状态
HLE_S32 cmd_request_login(HLE_S32 SessionID); 				// 登陆请求命令
HLE_S32 cmd_set_audio_vol(HLE_S32 SessionID); 				//设置AUdio音量参数
HLE_S32 cmd_set_time_zone(HLE_S32 SessionID);						//时区设置（校时）



#endif






