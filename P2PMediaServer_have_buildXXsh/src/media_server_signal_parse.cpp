/*********************************************************************************
  *FileName: media_server_signal_parse.cpp
  *Create Date: 2018/09/18
  *Description: 客户端信令解析的头文件，用户事件处理函数 。 
  *Others:  
  *History:  
**********************************************************************************/

#include <iostream>
#include <string.h>
#include <stdio.h>

#include "media_server_signal_def.h"
#include "media_server_interface.h"
#include "plog.h"


//获取设备MAC地址
int cmd_Get_Mac(void*data,int length)
{
	plog("cmd_Get_Mac sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return media_ser_success;
}

// 读设备信息	
int cmd_read_dev_info(void*data,int length)
{
	plog("cmd_read_dev_info sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return media_ser_success;
}

//设置设备系统参数	
int cmd_set_dev_para(void*data,int length)
{
	plog("cmd_set_dev_para sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return media_ser_success;
}

//读取设备系统参数	
int cmd_read_dev_para(void*data,int length)
{
	plog("cmd_read_dev_para sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return media_ser_success;
}

//报警通知
int cmd_alarm_update(void*data,int length)
{
	plog("cmd_alarm_update sucess!\n");
	return media_ser_success;
}

//打开实时流传输
int cmd_open_living(void*data,int length)
{
	plog("cmd_open_living sucess!\n");
	return media_ser_success;
}

//关闭实时流传输
int cmd_close_living(void*data,int length)
{
	plog("cmd_close_living sucess!\n");
	return media_ser_success;
}

//重启命令
int cmd_set_reboot(void*data,int length)
{
	plog("cmd_set_reboot sucess!\n");
	return media_ser_success;
}

//升级命令
int cmd_set_update(void*data,int length)
{
	plog("cmd_set_update sucess!\n");
	return media_ser_success;
}

//连接WIFI热点		
int cmd_set_wifi_connect(void*data,int length)
{
	plog("cmd_set_wifi_connect sucess!\n");
	return media_ser_success;
}

//获取wifi连接状态
int cmd_get_wifi_status(void*data,int length)
{
	plog("cmd_get_wifi_status sucess!\n");
	return media_ser_success;
}

// 登陆请求命令
int cmd_request_login(void*data,int length)
{
	plog("cmd_request_login sucess!\n");
	return media_ser_success;
}

//设置AUdio音量参数
int cmd_set_audio_vol(void*data,int length)
{
	plog("cmd_set_audio_vol sucess!\n");
	return media_ser_success;
}



/*
	传入接收到的信令数据，该函数解析接收到客户端发送的信令，交给响应函数处理
*/
int med_ser_signal_parse(void*data,int length)
{

	if(NULL == data)
	{
		plogerr("illegal argument!!\n");
		return media_ser_param_err;
	}
	
	cmd_header_t * cmd_header = (cmd_header_t*)data;
	if(cmd_header->head != HLE_MAGIC)
	{
		plogerr("cmd header is illegal!\n");
		return media_ser_error;
	}

	/*数据长度校验*/
	if(cmd_header->length + (int)sizeof(cmd_header_t) != length)
	{
		plogerr("package data length error!\n");
		return media_ser_error;
	}
	
	/*cmd_header->type要不要做处理？？用来干什么*/
	switch(cmd_header->command)
	{
		case CMD_GET_MAC :    		//获取设备MAC地址
			/*响应函数*/
			cmd_Get_Mac(data,length);	
			break;

	
		case CMD_READ_DEV_INFO:		// 读设备信息	
				/*响应函数*/
			cmd_read_dev_info(data,length);
			break;
		case CMD_SET_DEV_PARA:		//设置设备系统参数
				/*响应函数*/
			cmd_set_dev_para(data,length);
			break;
		
		
		case CMD_READ_DEV_PARA:		// 读取设备系统参数	
			/*响应函数*/
			cmd_read_dev_para(data,length);
			break;
		
		case CMD_ALARM_UPDATE:		//报警通知
			/*响应函数*/
			cmd_alarm_update(data,length);
			break;

		case CMD_OPEN_LIVING:		//打开实时流传输
				/*响应函数*/
			cmd_open_living(data,length);
			break;
				
		case CMD_CLOSE_LIVING:		//关闭实时流传输
				/*响应函数*/
			cmd_close_living(data,length);
			break;
		case CMD_SET_REBOOT:			// 重启命令
				/*响应函数*/
			cmd_set_reboot(data,length);
			break;
		
		case CMD_SET_UPDATE:			//升级命令
				/*响应函数*/
			cmd_set_update(data,length);
			break;
		
		case CMD_SET_WIFI_CONNECT:		//连接WIFI热点
				/*响应函数*/
			cmd_set_wifi_connect(data,length);
			break;
				
		case CMD_GET_WIFI_STATUS:		//获取wifi连接状态
				/*响应函数*/
			cmd_get_wifi_status(data, length);
			break;
				
		case CMD_REQUEST_LOGIN:			// 登陆请求命令
				/*响应函数*/
			cmd_request_login(data,length); 
			break;
				
		case CMD_SET_AUDIO_VOL:			//设置AUdio音量参数
				/*响应函数*/
			cmd_set_audio_vol(data, length);
			break;
				
		default:
				/*响应函数*/
			plog("default command!\n");
			break;
				
	}
	

	return media_ser_success;
}




