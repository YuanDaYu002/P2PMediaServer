#include <stdio.h>
#include <time.h>
#include "timezone.h"


/******************************************
功能：返回本地时间所对应的时区
******************************************/
int local_timezone()
{
	int timezone = 0;
    time_t t1, t2 ;
    struct tm *tm_local, *tm_utc;
 
	time(&t1);
	t2 = t1;
	printf("t1=%lu,t2=%lu\n", t1, t2);
	
	tm_local = localtime(&t1);
	printf("localtime=%d:%d:%d\n", tm_local->tm_hour, tm_local->tm_min, tm_local->tm_sec);
	t1 = mktime(tm_local) ;//将本地时间结构数据转换成经过的秒数(距1970年1月1日0:0:0)返回秒数

	tm_utc = gmtime(&t2);//获取UTC时间(世界时间、标准时间UTC,格林威治)
	printf("utcutctime=%d:%d:%d\n", tm_utc->tm_hour, tm_utc->tm_min, tm_utc->tm_sec);
	t2 = mktime(tm_utc);//将UTC时间结构数据转换成经过的秒数(距1970年1月1日0:0:0)返回秒数
	printf("***\n");
	printf("t1=%lu\nt2=%lu\n", t1, t2);
	
    timezone = (t1 - t2) / 3600;
	//printf("%d\n", timezone);


	return timezone;
}

/******************************************
功能：依据时区自动校时的函数。
******************************************/
int sync_timezone(unsigned char city_num)
{
	if(city_num<0||city_num>142)
		return -1;

	time_t mytime;//time_t实际上就是一个long int整型
	struct tm * mystruct = NULL;
	
	/*获取日历时间*/
	mytime = time(NULL);
	
	/*调整时区前,转换为本地时间*/
	mystruct = localtime(&mytime);
	
	printf("Before adjust timezone :year:%d month:%d day:%d hour:%d minute:%d second:%d\n",\
			mystruct->tm_year,mystruct->tm_mon+1,mystruct->tm_mday,mystruct->tm_hour,\
			mystruct->tm_min,mystruct->tm_sec);
	/*****************************************************************************/
	time_t UTC_time,new_zone_time;//time_t实际上就是一个long int整型
	struct tm *tm_utc;
	tm_utc = gmtime(&UTC_time);//获取UTC时间(世界时间、标准时间UTC,格林威治)
	printf("UTC time=%d:%d:%d\n", tm_utc->tm_hour, tm_utc->tm_min, tm_utc->tm_sec);
	UTC_time = mktime(tm_utc);//将UTC时间结构数据转换成经过的秒数(距1970年1月1日0:0:0)返回秒数

	new_zone_time = UTC_time + tz_diff_table[city_num].diff_timezone;

	
	/*调整时区后,转换为本地时间*/
	mystruct = localtime(&new_zone_time);
	
	printf("After adjust timezone :year:%d month:%d day:%d hour:%d minute:%d second:%d\n",\
			mystruct->tm_year,mystruct->tm_mon+1,mystruct->tm_mday,mystruct->tm_hour,\
			mystruct->tm_min,mystruct->tm_sec);
	
	/*lafter ,set time to system,all right*/

}


int main()
{
	float A ;
	A = +1.5;
	printf("A = %f\n",A);
	

	

	
	int timezone = local_timezone();
	printf("timezone = %d\n",timezone);

	sync_timezone(43);

	return 0;
	
}

