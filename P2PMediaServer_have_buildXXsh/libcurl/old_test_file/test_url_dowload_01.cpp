#include <stdio.h>
#include <curl/curl.h>
#include "curl.h"
#include "easy.h"


#define FALSE -1

size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  return fwrite(ptr, size, nmemb, stream);
} 
 
int my_progress_func(char *progress_data,
                     double t, /* dltotal */
                     double d, /* dlnow */
                     double ultotal,
                     double ulnow)
{
  printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);
  return 0;
}
 
int main(int argc, char **argv)
{
  CURL *curl;
  CURLcode res;
  FILE *outfile;
//  char *url = "http://10.10.1.4/d/c00000000000039/2014-10-22/10-28-35.ps";
  char *url = "https://pan.baidu.com/s/1XYETNarNF6EPM579vOyNEg";
  char *progress_data = "* ";
 
  curl = curl_easy_init();
  if(curl)
  {
    outfile = fopen("test.txt", "wb");
 
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
 
    res = curl_easy_perform(curl);
 
    fclose(outfile);
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}
