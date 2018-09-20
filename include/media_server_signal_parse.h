/*********************************************************************************
  *FileName: media_server_signal_parse.h
  *Create Date: 2018/09/19
  *Description: 客户端信令解析的头文件，用户事件处理函数 
  *Others:  
  *History:  
**********************************************************************************/
#ifndef MEDIA_SERVER_SIGNAL_PARSE_H
#define MEDIA_SERVER_SIGNAL_PARSE_H

int med_ser_signal_parse(void*data,int length);

int cmd_Get_Mac(void*data,int length);						//获取设备MAC地址
int cmd_read_dev_info(void*data,int length);				//读设备信息	
int cmd_set_dev_para(void*data,int length);  				//设置设备系统参数	
int cmd_read_dev_para(void*data,int length); 				//读取设备系统参数	
int cmd_alarm_update(void*data,int length); 				//报警通知
int cmd_open_living(void*data,int length); 					//打开实时流传输
int cmd_close_living(void*data,int length); 				//关闭实时流传输
int cmd_set_reboot(void*data,int length); 					//重启命令
int cmd_set_update(void*data,int length); 					//升级命令
int cmd_set_wifi_connect(void*data,int length); 			//连接WIFI热点		
int cmd_get_wifi_status(void*data,int length); 				//获取wifi连接状态
int cmd_request_login(void*data,int length); 				// 登陆请求命令
int cmd_set_audio_vol(void*data,int length); 				//设置AUdio音量参数


#endif



