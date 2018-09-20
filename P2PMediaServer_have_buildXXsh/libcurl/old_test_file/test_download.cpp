/*==============================================
# FileName: test_download.cpp
# Version: 1.0
# LastChange: 2010-03-09 14:20:44
# Description:
# History:
============================================*/
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>

#include "curl.h"


using namespace std;
static char errorBuffer[CURL_ERROR_SIZE];
static int writer(char *, size_t, size_t, string *);
static bool init(CURL *&, char *,string *);

int main()
{
	CURL *conn = NULL;
	CURLcode code;
	string buffer;
	
	curl_global_init(CURL_GLOBAL_DEFAULT);
	char* url="172.16.211.50/cc2/cc/getfile.php";
	if (!init(conn,url,&buffer ))
	{
		fprintf(stderr, "Connection initializion failed\n");
		exit(EXIT_FAILURE);
	}
	code = curl_easy_perform(conn);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to get '%s' [%s]\n", url, errorBuffer);
		exit(EXIT_FAILURE);
	}
	FILE * file = fopen("1.gif","wb");
	fseek(file,0,SEEK_SET);
	fwrite(buffer.c_str(),1,buffer.size(),file);
	fclose(file);
	curl_easy_cleanup(conn);
	printf("%s\n",buffer.c_str());
	
	return 0;
}
static bool init(CURL *&conn, char *url,string *p_buffer)
{
	CURLcode code;
	conn = curl_easy_init();
	if (conn == NULL)
	{
		fprintf(stderr, "Failed to create CURL connection\n");
		exit(EXIT_FAILURE);
	}
	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set error buffer [%d]\n", code);
		return false;
	}
	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
		return false;
	}
	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
		return false;
	}
	code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);
		return false;
	}
	code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, p_buffer);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);
		return false;
	}
	return true;
}
static int writer(char *data, size_t size, size_t nmemb, string *writerData)
{
	unsigned long sizes = size * nmemb;
	if (writerData == NULL) return 0;
	writerData->append(data, sizes);
	return sizes;
}
