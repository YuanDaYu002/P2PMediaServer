/*********************************************************************************
  *FileName: media_server.cpp
  *Create Date: 2018/09/18
  *Description: P2P media server 的主干业务文件，也是P2P media server的入口文件。 
  *Others:  
  *History:  
**********************************************************************************/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "media_server_p2p.h"
#include "media_server_signal_def.h"
#include "media_server_interface.h"
#include "plog.h"
#include "media_server.h"




/*
int media_server_init()
{

}

*/

//int media_server_start()//调试成功后将main直接改成该函数就好
char is_first_run = 0;
int main(int argc,char**argv) 
{
	
	is_first_run = 1; 
	int ret;
	
	/*创建IDLE线程*/
	ret = Idle_thread_create();
	if(ret < 0)
	{
		plogerr("Idle_thread_create error !\n");
		return media_ser_error;	
	}
	plog("Idle_thread_create success!\n");
	
	
	p2p_handle_t P2P_handle;
	memset(&P2P_handle,0,sizeof(p2p_handle_t));


	ret = p2p_init(&P2P_handle);
	if(ret < 0)
	{
		plogerr("p2p_init error !\n");
		return media_ser_error;
	}
	plog("p2p_init success!\n");


	ret = p2p_conect(&P2P_handle);
	if(ret < 0)
	{
		plogerr("p2p_conect error !\n");
		return media_ser_error;	
	}
	plog("p2p_conect success!\n");


	while(1)//后边要有条件，不能是死循环。
	{
		do
		{
			
			ret = P2P_wait_for_wakeup(&P2P_handle);
			
			
		}while(ret != 0 && ret != -2 && ret != -3);//ret = 0，即为唤醒,-2:不支持唤醒，-3：还有客户端没退出。
		plog("media server is wakeup !\n");
		do
		{
			ret = p2p_listen(&P2P_handle);
			//if()什么条件下需要重新进入到睡眠状态，3min没有客户端连接成功，自动breaK再进入睡眠就好了
			
			
		}while(ret < 0);
		
		
		if(ret >= 0 )//成功连接进客户端
		{
			plog("one client connect,SessionID:%d\n",ret);
			P2P_handle.SessionID = ret;
			P2P_client_task_create(&P2P_handle);//创建与客户端交互的业务线程。
		}
			
		
	}

		
		
    return media_ser_success;
}

/***************************************************
闲置线程，负责一些比较杂的业务，其中就包括了校时功能
***************************************************/
void* Idle(void*arg)
{

	time_t mytime;//time_t实际上就是一个整型
	struct tm * mystruct = NULL;



	while(1)

	{
		
		
		/*获取日历时间*/
		mytime = time(NULL);
		printf("mytime = %ld\n",mytime);
		
		/*转换为本地时间*/
		mystruct = localtime(&mytime);
		#if 1	
			printf("year:%d month:%d day:%d hour:%d minute:%d second:%d\n",\
				mystruct->tm_year,mystruct->tm_mon+1,mystruct->tm_mday,mystruct->tm_hour,\
				mystruct->tm_min,mystruct->tm_sec);
		#endif
		/*创建校时线程,条件: 1.设备启动时进行一次校时操作 2.每天凌晨0点进行一次校时操作*/

		if(1 == is_first_run || (0 == mystruct->tm_hour&& 0 == mystruct->tm_min && 0== mystruct->tm_sec))
		{
			is_first_run = 0;
			pthread_t threadID_Ntp_sync_time;
		#if 1
			int err = pthread_create(&threadID_Ntp_sync_time, NULL, &ntp_sync_time_func, NULL);
			if (0 != err) 
			{
				plogerr("create Thread Ntp_sync_time failed!\n");
			}
		#endif

		}


		usleep(100);
		return NULL;
		

	}
	
}

/***************************************************
创建闲置线程
***************************************************/
int Idle_thread_create(void)
{
	pthread_t threadID_Idle;
	int err = pthread_create(&threadID_Idle, NULL, &Idle, NULL);
	if (0 != err) 
	{
		plogerr("create Thread Idle failed!\n");
	}
	
	return err;
}



