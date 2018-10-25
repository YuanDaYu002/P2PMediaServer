#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "write_file_to_flash.h"
#include "spinor.h"

#include <stdlib.h>//不加会报隐式声明与内建函数'exit'不兼容
#include <sys/socket.h>
#include <netinet/in.h>//因用了地址

/*******************************************************************
手动烧写文件到norflash，
从设备文件系统中读取一个系统文件（ipc8.bin）烧到norflash的系统位置，
********************************************************************/
//argv[1]:要读取的源文件
int  write_file_to_nor_flash(int argc,char**argv)
{
	char need_help = !strcmp(argv[0],"-h")||!strcmp(argv[0],"-help");
	if(argc < 1||need_help)
	{
		
		printf("usage: WriteNorFlash  [inputfile]\n\
			Atteion : inputfile need absolute path\n\
			eg : WriteNorFlash /jffs0/ipc18.bin  \n");
		return 0;
	}
	//printf("argv[0] = %s \n",argv[0]);
	int read_file = open((const char *)argv[0], O_RDONLY);
	if( read_file < 0)
	{
		
		perror("open read_file  error !\n");
		return -1;
	}

	/*分配缓存空间*/
	char * read_buffer = (char*)malloc(ERRASE_SIZE);
	if(NULL == read_buffer)
	{
		perror("malloc failed !\n");
		return -1;
	}
	memset(read_buffer,0,ERRASE_SIZE);

	/*读取文件至缓存空间*/
	unsigned long size = read(read_file,read_buffer,ERRASE_SIZE);
	printf("read size = %d bytes\n",size);

	/*擦除NorFlash*/
	printf("erase NorFlash......\n");
	int ret = hispinor_erase(FLASH_START_ADDRESS, ERRASE_SIZE);

	/*写NorFlash*/
	printf("write NorFlash......\n");
	ret = hispinor_write(read_buffer, FLASH_START_ADDRESS, size);
	if(0 == ret)
	{
		printf("write NroFlash success ,write(%d)bytes!\n",size);
	}
	else
	{
		perror("write NroFlash failed !\n");
	}
	

	free(read_buffer);
	close(read_file);
	return 0;
}

/*******************************************************************
功能：接收文件到内存并直接烧写到norflash （TCP传输）：
	专门烧写系统文件（ipc18.bin）用，提高开发调试速度，比ftp传输文件
	再烧写的方式要快，因接收的数据不保存成文件，而是直接从内存写入norflash
使用：配合客户端程序 send_and_write_file_to_norflash 使用
	1.设备端运行命令“RecvWriteNor”（会调用该函数）等待接收数据；
	2.虚拟机（linux主机）下运行客户端程序 send_and_write_file_to_norflash
		格式：send_and_write_file_to_norflash [file name] [deviceIP]
		eg： ./send_and_write_file_to_norflash ipc18.bin 192.168.3.82
注意：目前没有加入文件校验机制
*******************************************************************/
#define portnum  5555  //端口有可能端口被占用，可以改端口才能通
int  receive_and_write_file_to_nor_flash(int argc,char**argv)
{

	char need_help = !strcmp(argv[0],"-h")||!strcmp(argv[0],"-help");
	if(argc !=0||need_help)
	{
		
		printf("usage: RecvWriteNor\n\
			Atteion : need to be used in conjunction with client process: send_and_write_file_to_norflash]\n\
			eg : \n\
			board: RecvWriteNor\n\
			linux computer: send_and_write_file_to_norflash [file name] [deviceIP]\n");
		return 0;
	}

	int sockfd;
	int new_fd;//建立连接后会返回一个新的fd
	struct sockaddr_in sever_addr; //服务器的IP地址
	struct sockaddr_in client_addr; //客户机的IP地址
	char *buffer = (char*)malloc(ERRASE_SIZE);
	if(NULL == buffer)
	{
		perror("malloc failed!\n");
		return -1;
	}
	char *p_buffer = buffer;
	memset(buffer,0,ERRASE_SIZE);
	int nbyte;
	int checkListen;
	unsigned int filesize = 0;
	unsigned int countbytes = 0;
	
	/***为了消除accept函数第3个参数的类型不匹配问题的警告***********/
	int sockaddr_size = sizeof(struct sockaddr);
	/******************************************************/
	
	//1.创建套接字
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("create socket error!\n");
		return -1;
	}
	printf("create socket success!\n");

	//2.1设置要绑定的服务器地址
	bzero(&sever_addr,sizeof(struct sockaddr_in));
	sever_addr.sin_family = AF_INET;
	sever_addr.sin_port = htons(portnum);//将主机字节序转换成网络字节序
	sever_addr.sin_addr.s_addr = htonl(INADDR_ANY);//绑定任意的IP地址，要和网络上所有的IP地址通讯
	
	
	//2.2绑定地址
	if(bind(sockfd,(struct sockaddr*)&sever_addr,sizeof(struct sockaddr)) < 0)
	{
		printf("bind socket error!\n");
		return -1;
	}
	printf("bind socket success!\n");
	
	//3.监听端口
	printf("into listen portnum(%d)......!\n",portnum);
	checkListen = listen(sockfd,5);
	if(checkListen < 0)
	{
		perror("socket listen error!\n");
		return -1;
	}
	
	do
	{
		//4.等待连接
		new_fd = accept(sockfd,(struct sockaddr*)(&client_addr),(socklen_t *)(&sockaddr_size));
		if(new_fd<0)
		{
			printf("accept eror!\n");
			return -1;
		}
		printf("sever get connection from %s\n",inet_ntoa(client_addr.sin_addr.s_addr));
		
		//5.接收文件大小信息
		recv(new_fd,&filesize,sizeof(filesize),0);
		printf("receive file size = %ld bytes\n",filesize);
		if(filesize <= 0)return -1;

		//5.1接收数据
		while(1)
		{
			if(countbytes >= filesize)break;
			nbyte = recv(new_fd,p_buffer+countbytes,filesize - countbytes,0);
			countbytes = countbytes + nbyte;
			//printf("countbytes = %d bytes...\n",countbytes);
			
		}
		printf("success ! server received (%d)bytes!\n",countbytes);

	
	}while(0);

	/*****debug code ***/
	#if 0
		int tmp_file = open("/jffs0/tmp.bin", O_RDWR| O_CREAT,0660);
		if( tmp_file < 0)
		{
			
			perror("open read_file  error !\n");
			return -1;
		}
		printf("open read_file  success !\n");

	//debug
		printf("receive:%s \n",buffer);
		unsigned int tmp_ret = write(tmp_file, buffer, countbytes);
		if(tmp_ret < 0)
		{
			perror("write file error!\n");
			return -1;
		}
		printf("write file success! write size(%ld)bytes! \n",tmp_ret);

	#endif

	/*******************/
#if 1
	/*擦除NorFlash*/
	if(countbytes != filesize)
	{
		printf("countbytes != filesize\n");
		return -1;
	}

	printf("erase NorFlash......\n");
	int ret = hispinor_erase(FLASH_START_ADDRESS, ERRASE_SIZE);

	/*写NorFlash*/
	printf("write NorFlash......\n");
	ret = hispinor_write(buffer, FLASH_START_ADDRESS,countbytes);
	if(0 == ret)
	{
		printf("write NroFlash success ,write(%d)bytes!\n",countbytes);
	}
	else
	{
		perror("write NroFlash failed !\n");
	}

#endif

	close(new_fd);
	close(sockfd);
	free(buffer);

	return 0;
}
