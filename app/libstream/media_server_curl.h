/*********************************************************************************
  *FileName: media_server_curl.h
  *Create Date: 2018/09/30
  *Description: curl 头文件
  *Others:  
  *History:  
**********************************************************************************/
#ifndef MEDIA_SERVER_CURL_H
#define MEDIA_SERVER_CURL_H
#include "typeport.h"
#include "curl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FGETSFILE "fgets.test"   //下载文件储存的路径 待定


enum fcurl_type_e 
{
  CFTYPE_NONE = 0,
  CFTYPE_FILE = 1,
  CFTYPE_CURL = 2
};

struct fcurl_data
{
  enum fcurl_type_e type;     /* type of handle */
  union 
  {
    CURL *curl;
    FILE *file;
  } handle;                   /* handle */

  char *buffer;               /* buffer to store cached data*/
  size_t buffer_len;          /* currently allocated buffers length */
  size_t buffer_pos;          /* end of data in buffer*/
  int still_running;          /* Is background url fetch still in progress */
};

typedef struct fcurl_data URL_FILE;

/* exported functions */
URL_FILE *url_fopen(const char *url, const char *operation);
int url_fclose(URL_FILE *file);
int url_feof(URL_FILE *file);
size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
char *url_fgets(char *ptr, size_t size, URL_FILE *file);
void url_rewind(URL_FILE *file);


int url_download_file(char *url);


#ifdef __cplusplus
}
#endif

#endif


