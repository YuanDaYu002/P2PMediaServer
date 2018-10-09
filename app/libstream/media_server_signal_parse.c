/*********************************************************************************
  *FileName: media_server_signal_parse.c
  *Create Date: 2018/09/18
  *Description: 客户端信令解析的头文件，用户事件处理函数 。 
  *Others:  
  *History:  
**********************************************************************************/

#include <string.h>
#include <stdio.h>
#include <time.h>


#include "media_server_signal_def.h"
#include "media_server_interface.h"
#include "typeport.h"
#include "media_server_p2p.h"

#include "PPCS_API.h"
#include "PPCS_Error.h"
#include "PPCS_Type.h"
#include "opt.h"
#include "timezone.h"
#include "media_server_curl.h"



//获取设备MAC地址
HLE_S32 cmd_Get_Mac(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_Get_Mac sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return HLE_RET_OK;
}

// 读设备信息	
HLE_S32 cmd_read_dev_info(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_read_dev_info sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return HLE_RET_OK;
}

//设置设备系统参数	
HLE_S32 cmd_set_dev_para(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_set_dev_para sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return HLE_RET_OK;
}

//读取设备系统参数	
HLE_S32 cmd_read_dev_para(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_read_dev_para sucess!\n");
	/*需要知道获取设备参的接口和类型*/
	return HLE_RET_OK;
}

//报警通知
HLE_S32 cmd_alarm_update(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_alarm_update sucess!\n");
	return HLE_RET_OK;
}

//打开实时流传输
HLE_S32 cmd_open_living(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_open_living sucess!\n");
	return HLE_RET_OK;
}

//关闭实时流传输
HLE_S32 cmd_close_living(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_close_living sucess!\n");
	return HLE_RET_OK;
}

//重启命令
HLE_S32 cmd_set_reboot(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_set_reboot sucess!\n");
	return HLE_RET_OK;
}

//升级命令
HLE_S32 cmd_set_update(HLE_S32 SessionID)
{
	char * url = NULL;
	url_download_file(url);
	DEBUG_LOG("cmd_set_update sucess!\n");
	return HLE_RET_OK;
}

//连接WIFI热点		
HLE_S32 cmd_set_wifi_connect(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_set_wifi_connect sucess!\n");
	return HLE_RET_OK;
}

//获取wifi连接状态
HLE_S32 cmd_get_wifi_status(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_get_wifi_status sucess!\n");
	return HLE_RET_OK;
}

// 登陆请求命令
HLE_S32 cmd_request_login(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_request_login sucess!\n");
	return HLE_RET_OK;
}

//设置AUdio音量参数
HLE_S32 cmd_set_audio_vol(HLE_S32 SessionID)
{
	DEBUG_LOG("cmd_set_audio_vol sucess!\n");
	return HLE_RET_OK;
}

//设置时区(校时)命令
HLE_S32 cmd_set_time_zone(HLE_S32 SessionID)
{
	
	STRUCT_SET_TIME_ZONE_REQUEST data_buf;
	HLE_S32  BufSize = sizeof(STRUCT_SET_TIME_ZONE_REQUEST);

	HLE_S32 ret = PPCS_Read(SessionID, CH_CMD, (CHAR*)&data_buf, &BufSize, 2000);
	if(ret < 0)//读取出错
	{
		if (ERROR_PPCS_TIME_OUT == ret) 
		{
			ERROR_LOG("PPCS_Read timeout!!\n");
			
		}	
		else if (ERROR_PPCS_SESSION_CLOSED_REMOTE == ret) 
		{
			ERROR_LOG("Remote site call close!!\n");
			
		}
		else 
		{
			ERROR_LOG("PPCS_Read unknow error!ret=%d\n", ret);
			
			
		}
		return HLE_RET_ERROR;

	}
	
	DEBUG_LOG("Read TimeZone data success! BufSize(%d) ReadSize(%d)\n",BufSize,ret);

	
	time_t settime, difftime;		//time_t实际上就是一个long int型
	settime = time(NULL);
	
	/*1.计算新时区应设置的时间秒数*/
	if(data_buf.tzindex < 0||data_buf.tzindex > 142)
	{
		ERROR_LOG("tzindex out of range!\n");
		return HLE_RET_EINVAL;
	}
	city_time_zone_t timezone_buf =  tz_diff_table[data_buf.tzindex];
	difftime = timezone_buf.diff_timezone; //新设置的时区与UTC时间差值查询
	settime = settime + difftime;	

	/*2.进行夏令时调整*/
	if(data_buf.daylight)
	{
		/*南北半球国家的夏令时调整时间不统一，且还存在法律调整等情况，
		所以不方便用程序将夏令时的调整时间固定下来，因此夏令时调整时，
		需要用户通过手机app进行一次校时操作(打开夏令时开关)*/
		settime = settime + 3600;
	}

	/*3.重新设置系统rtc时钟*/
	SNTP_SET_SYSTEM_TIME(settime);/** SNTP macro to change system time and/or the update the RTC clock */

	return HLE_RET_OK;
}


/*
	传入接收到的信令数据，该函数解析接收到客户端发送的信令，交给响应函数处理
*/
HLE_S32 med_ser_signal_parse(HLE_S32 SessionID,void*data,HLE_S32 length)
{

	if(NULL == data)
	{
		ERROR_LOG("illegal argument!!\n");
		return HLE_RET_EINVAL;
	}
	
	cmd_header_t * cmd_header = (cmd_header_t*)data;
	if(cmd_header->head != HLE_MAGIC)
	{
		ERROR_LOG("cmd header is illegal!\n");
		return HLE_RET_ERROR;
	}

	/*数据长度校验*/
	if(cmd_header->length + (HLE_S32)sizeof(cmd_header_t) != length)
	{
		ERROR_LOG("package data length error!\n");
		return HLE_RET_ERROR;
	}
	
	/*cmd_header->type暂时没使用*/

	
	switch(cmd_header->command)
	{
		case CMD_GET_MAC :    		//获取设备MAC地址
			/*响应函数*/
			cmd_Get_Mac(SessionID);	
			break;

	
		case CMD_READ_DEV_INFO:		// 读设备信息	
				/*响应函数*/
			cmd_read_dev_info(SessionID);
			break;
		case CMD_SET_DEV_PARA:		//设置设备系统参数
				/*响应函数*/
			cmd_set_dev_para(SessionID);
			break;
		
		
		case CMD_READ_DEV_PARA:		// 读取设备系统参数	
			/*响应函数*/
			cmd_read_dev_para(SessionID);
			break;
		
		case CMD_ALARM_UPDATE:		//报警通知
			/*响应函数*/
			cmd_alarm_update(SessionID);
			break;

		case CMD_OPEN_LIVING:		//打开实时流传输
				/*响应函数*/
			cmd_open_living(SessionID);
			break;
				
		case CMD_CLOSE_LIVING:		//关闭实时流传输
				/*响应函数*/
			cmd_close_living(SessionID);
			break;
		case CMD_SET_REBOOT:			// 重启命令
				/*响应函数*/
			cmd_set_reboot(SessionID);
			break;
		
		case CMD_SET_UPDATE:			//升级命令
				/*响应函数*/
			cmd_set_update(SessionID);
			break;
		
		case CMD_SET_WIFI_CONNECT:		//连接WIFI热点
				/*响应函数*/
			cmd_set_wifi_connect(SessionID);
			break;
				
		case CMD_GET_WIFI_STATUS:		//获取wifi连接状态
				/*响应函数*/
			cmd_get_wifi_status(SessionID);
			break;
				
		case CMD_REQUEST_LOGIN:			// 登陆请求命令
				/*响应函数*/
			cmd_request_login(SessionID); 
			break;
				
		case CMD_SET_AUDIO_VOL:			//设置AUdio音量参数
				/*响应函数*/
			cmd_set_audio_vol(SessionID);
			break;
		case CMD_SET_TIME_ZONE:			//设置时区(校时)命令
			cmd_set_time_zone(SessionID);
			break;
		default:
				/*响应函数*/
			DEBUG_LOG("default command!\n");
			break;
				
	}
	

	return HLE_RET_OK;
}









