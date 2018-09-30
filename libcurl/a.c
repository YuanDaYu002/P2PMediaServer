
#include <stdio.h>
#include "./curl-7.61.1/include/curl/curl.h"
//#include "./curl-7.61.1/include/curl/curl.h"

int main() 
{
	printf("%s\n", curl_version());
	return 0;
}
