#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "write_file_to_flash.h"
#include "spinor.h"

//argv[1]:要读取的源文件
int  write_file_to_nor_flash(int argc,char**argv)
{
	if(argc < 1)
	{
		
		printf("ussage: WriteNorFlash + inputfile(need absolute path)\
			\neg : WriteNorFlash /jffs0/ipc18.bin  \n");
		return -1;
	}
	printf("argv[0] = %s \n",argv[0]);
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
	int ret = hispinor_erase(FLASH_START_ADDRESS, ERRASE_SIZE);

	/*写NorFlash*/
	ret = hispinor_write(read_buffer, FLASH_START_ADDRESS, size);
	if(0 == ret)
	{
		printf("write NroFlash success !\n");
	}
	else
	{
		perror("write NroFlash failed !\n");
	}
	

	free(read_buffer);
	close(read_file);
	return 0;
}
