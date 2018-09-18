#include <iostream>
#include <string.h>
#include <stdio.h>
#include "media_server_p2p.h"
#include "media_server_signal_def.h"

int main(int argc,char**argv) 
{

	p2p_handle_t P2P_handle;
	memset(&P2P_handle,0,sizeof(p2p_handle_t));
	
	int ret = p2p_init(&P2P_handle);
	if(ret < 0)
	{
		printf("p2p_init error !\n");
		return -1;
	}
	
	ret = p2p_conect(&P2P_handle);
	if(ret < 0)
	{
		printf("p2p_conect error !\n");
		return -1;	
	}

	while(1)//后边要有条件，不能是死循环。
	{
		do
		{
			
			ret = P2P_wait_for_wakeup(&P2P_handle);
			
			
		}while(ret != 0 && ret != -2 && ret != -3);//ret = 0，即为唤醒,-2:不支持唤醒，-3：还有客户端没退出。
		
		do
		{
			printf("into p2p_listen\n");
			ret = p2p_listen(&P2P_handle);
			//if()什么条件下需要重新进入到睡眠状态，3min没有客户端连接成功，自动breaK再进入睡眠就好了
			
			
		}while(ret < 0);
			
		if(ret >= 0 )//成功连接进客户端
		{
			P2P_handle.SessionID = ret;
			printf("into P2P_client_task_create\n");
			P2P_client_task_create(&P2P_handle);//创建与客户端交互的业务线程。
		}
			
		
	}

		
		
    return 0;
}

