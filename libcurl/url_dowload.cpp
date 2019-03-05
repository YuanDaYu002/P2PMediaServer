/*****************************************************************************
 *
 * This example source code introduces a c library buffered I/O interface to
 * URL reads it supports fopen(), fread(), fgets(), feof(), fclose(),
 * rewind(). Supported functions have identical prototypes to their normal c
 * lib namesakes and are preceaded by url_ .
 *
 * Using this code you can replace your program's fopen() with url_fopen()
 * and fread() with url_fread() and it become possible to read remote streams
 * instead of (only) local files. Local files (ie those that can be directly
 * fopened) will drop back to using the underlying clib implementations
 *
 * See the main() function at the bottom that shows an app that retrieves from
 * a specified url using fgets() and fread() and saves as two output files.
 *
 * Copyright (c) 2003, 2017 Simtec Electronics
 *
 * Re-implemented by Vincent Sanders <vince@kyllikki.org> with extensive
 * reference to original curl example code
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This example requires libcurl 7.9.7 or later.
 */
/* <DESC>
 * implements an fopen() abstraction allowing reading from URLs
 * </DESC>
 */

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#  include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <curl/curl.h>

enum fcurl_type_e {
  CFTYPE_NONE = 0,
  CFTYPE_FILE = 1,
  CFTYPE_CURL = 2
};

struct fcurl_data
{
  enum fcurl_type_e type;     /* type of handle */
  union {
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

/* we use a global one for convenience */
static CURLM *multi_handle;


/* curl calls this routine to get more data */
static size_t write_callback(char *buffer,
                             size_t size,
                             size_t nitems,
                             void *userp)
{
  char *newbuff;
  size_t rembuff;

  URL_FILE *url = (URL_FILE *)userp;
  size *= nitems;

  rembuff = url->buffer_len - url->buffer_pos; /* remaining space in buffer */

  if(size > rembuff) {
    /* not enough space in buffer */
    newbuff = (char*)realloc(url->buffer, url->buffer_len + (size - rembuff));
    if(newbuff == NULL) {
      fprintf(stderr, "callback buffer grow failed\n");
      size = rembuff;
    }
    else {
      /* realloc succeeded increase buffer size*/
      url->buffer_len += size - rembuff;
      url->buffer = newbuff;
    }
  }

  memcpy(&url->buffer[url->buffer_pos], buffer, size);
  url->buffer_pos += size;

  return size;
}

#if 1
/* use to attempt to fill the read buffer up to requested number of bytes */
static int fill_buffer(URL_FILE *file, size_t want)
{
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcep;
  struct timeval timeout;
  int rc;
  CURLMcode mc; /* curl_multi_fdset() return code */

  /* only attempt to fill buffer if transactions still running and buffer
   * doesn't exceed required size already
   */
  if((!file->still_running) || (file->buffer_pos > want))
    return 0;

  /* attempt to fill buffer */
  do {
    int maxfd = -1;
    long curl_timeo = -1;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to fail on */
    timeout.tv_sec = 60; /* 1 minute */
    timeout.tv_usec = 0;

    curl_multi_timeout(multi_handle, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    if(mc != CURLM_OK) {
      fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
      break;
    }

    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */

    if(maxfd == -1) {
#ifdef _WIN32
      Sleep(100);
      rc = 0;
#else
      /* Portable sleep for platforms other than Windows. */
      struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
      rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else {
      /* Note that on some platforms 'timeout' may be modified by select().
         If you need access to the original value save a copy beforehand. */
      rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    }

    switch(rc) {
    case -1:
      /* select error */
      break;

    case 0:
    default:
      /* timeout or readable/writable sockets */
      curl_multi_perform(multi_handle, &file->still_running);
      break;
    }
  } while(file->still_running && (file->buffer_pos < want));
  return 1;
}



/* use to remove want bytes from the front of a files buffer */
static int use_buffer(URL_FILE *file, size_t want)
{
  /* sort out buffer */
  if((file->buffer_pos - want) <= 0) {
    /* ditch buffer - write will recreate */
    free(file->buffer);
    file->buffer = NULL;
    file->buffer_pos = 0;
    file->buffer_len = 0;
  }
  else {
    /* move rest down make it available for later */
    memmove(file->buffer,
            &file->buffer[want],
            (file->buffer_pos - want));

    file->buffer_pos -= want;
  }
  return 0;
}

URL_FILE *url_fopen(const char *url, const char *operation)
{
  /* this code could check for URLs or types in the 'url' and
     basically use the real fopen() for standard files */

  URL_FILE *file;
  (void)operation;

  file = (URL_FILE*)calloc(1, sizeof(URL_FILE));
  if(!file)
    return NULL;

  file->handle.file = fopen(url, operation);
  if(file->handle.file)
    file->type = CFTYPE_FILE; /* marked as URL */

  else {
    file->type = CFTYPE_CURL; /* marked as URL */
    file->handle.curl = curl_easy_init();

    curl_easy_setopt(file->handle.curl, CURLOPT_URL, url);
    curl_easy_setopt(file->handle.curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(file->handle.curl, CURLOPT_WRITEFUNCTION, write_callback);

    if(!multi_handle)
      multi_handle = curl_multi_init();

    curl_multi_add_handle(multi_handle, file->handle.curl);

    /* lets start the fetch */
    curl_multi_perform(multi_handle, &file->still_running);

    if((file->buffer_pos == 0) && (!file->still_running)) {
      /* if still_running is 0 now, we should return NULL */

      /* make sure the easy handle is not in the multi handle anymore */
      curl_multi_remove_handle(multi_handle, file->handle.curl);

      /* cleanup */
      curl_easy_cleanup(file->handle.curl);

      free(file);

      file = NULL;
    }
  }
  return file;
}
#endif

#if 1
int url_fclose(URL_FILE *file)
{
  int ret = 0;/* default is good return */

  switch(file->type) {
  case CFTYPE_FILE:
    ret = fclose(file->handle.file); /* passthrough */
    break;

  case CFTYPE_CURL:
    /* make sure the easy handle is not in the multi handle anymore */
    curl_multi_remove_handle(multi_handle, file->handle.curl);

    /* cleanup */
    curl_easy_cleanup(file->handle.curl);
    break;

  default: /* unknown or supported type - oh dear */
    ret = EOF;
    errno = EBADF;
    break;
  }

  free(file->buffer);/* free any allocated buffer space */
  free(file);

  return ret;
}
#endif


int url_feof(URL_FILE *file)
{
  int ret = 0;

  switch(file->type) {
  case CFTYPE_FILE:
    ret = feof(file->handle.file);
    break;

  case CFTYPE_CURL:
    if((file->buffer_pos == 0) && (!file->still_running))
      ret = 1;
    break;

  default: /* unknown or supported type - oh dear */
    ret = -1;
    errno = EBADF;
    break;
  }
  return ret;
}

size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file)
{
  size_t want;

  switch(file->type) {
  case CFTYPE_FILE:
    want = fread(ptr, size, nmemb, file->handle.file);
    break;

  case CFTYPE_CURL:
    want = nmemb * size;

    fill_buffer(file, want);

    /* check if there's data in the buffer - if not fill_buffer()
     * either errored or EOF */
    if(!file->buffer_pos)
      return 0;

    /* ensure only available data is considered */
    if(file->buffer_pos < want)
      want = file->buffer_pos;

    /* xfer data to caller */
    memcpy(ptr, file->buffer, want);

    use_buffer(file, want);

    want = want / size;     /* number of items */
    break;

  default: /* unknown or supported type - oh dear */
    want = 0;
    errno = EBADF;
    break;

  }
  return want;
}

char *url_fgets(char *ptr, size_t size, URL_FILE *file)
{
  size_t want = size - 1;/* always need to leave room for zero termination */
  size_t loop;

  switch(file->type) {
  case CFTYPE_FILE:
    ptr = fgets(ptr, (int)size, file->handle.file);
    break;

  case CFTYPE_CURL:
    fill_buffer(file, want);

    /* check if there's data in the buffer - if not fill either errored or
     * EOF */
    if(!file->buffer_pos)
      return NULL;

    /* ensure only available data is considered */
    if(file->buffer_pos < want)
      want = file->buffer_pos;

    /*buffer contains data */
    /* look for newline or eof */
    for(loop = 0; loop < want; loop++) {
      if(file->buffer[loop] == '\n') {
        want = loop + 1;/* include newline */
        break;
      }
    }

    /* xfer data to caller */
    memcpy(ptr, file->buffer, want);
    ptr[want] = 0;/* always null terminate */

    use_buffer(file, want);

    break;

  default: /* unknown or supported type - oh dear */
    ptr = NULL;
    errno = EBADF;
    break;
  }

  return ptr;/*success */
}

#if 1
void url_rewind(URL_FILE *file)
{
  switch(file->type) {
  case CFTYPE_FILE:
    rewind(file->handle.file); /* passthrough */
    break;

  case CFTYPE_CURL:
    /* halt transaction */
    curl_multi_remove_handle(multi_handle, file->handle.curl);

    /* restart */
    curl_multi_add_handle(multi_handle, file->handle.curl);

    /* ditch buffer - write will recreate - resets stream pos*/
    free(file->buffer);
    file->buffer = NULL;
    file->buffer_pos = 0;
    file->buffer_len = 0;

    break;

  default: /* unknown or supported type - oh dear */
    break;
  }
}
#endif

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

#define FGETSFILE "fgets.test"
#define FREADFILE "fread.test"
#define REWINDFILE "rewind.test"

/* Small main program to retrieve from a url using fgets and fread saving the
 * output to two test files (note the fgets method will corrupt binary files if
 * they contain 0 chars */
int main(int argc, char *argv[])
{
  #if 0
  //原本的下载方式
      URL_FILE *handle;
      FILE *outf;

      size_t nread;
      char buffer[1024];
      const char *url;

      if(argc < 2)
        url = "https://qdcu01.baidupcs.com/file/96bb1f5e83aa621948f7d7f3b0ff8cf7?bkt=p3-140096bb1f5e83aa621948f7d7f3b0ff8cf7e210d6240000000001e8&fid=557564212-250528-965041864654337&time=1541469383&sign=FDTAXGERLQBHSKW-DCb740ccc5511e5e8fedcff06b081203-dkJWsB1QNsIN5Xs%2BExc8Q97UOIw%3D&to=65&size=488&sta_dx=488&sta_cs=4&sta_ft=txt&sta_ct=0&sta_mt=0&fm2=MH%2CQingdao%2CAnywhere%2C%2Cguangdong%2Cct&ctime=1541399159&mtime=1541399159&resv0=cdnback&resv1=0&vuk=557564212&iv=0&htype=&newver=1&newfm=1&secfm=1&flow_ver=3&pkey=140096bb1f5e83aa621948f7d7f3b0ff8cf7e210d6240000000001e8&sl=76480590&expires=8h&rt=sh&r=273911430&mlogid=7176187698743905372&vbdid=2678328550&fin=test_update.txt&fn=test_update.txt&rtype=1&dp-logid=7176187698743905372&dp-callid=0.1.1&hps=1&tsl=80&csl=80&csign=gK8PVmsTwsBYTJzeE6BWLbZWL60%3D&so=0&ut=6&uter=4&serv=0&uc=1694538685&ti=37cfb58296b21ea911bfb63a3d12cc3500bedcc7d5a1e95c&by=themis";/* default to testurl */
      else
        url = argv[1];/* use passed url */

    #if 1
      /* copy from url line by line with fgets */
      outf = fopen(FGETSFILE, "wb+");
      if(!outf) {
        perror("couldn't open fgets output file\n");
        return 1;
      }

      handle = url_fopen(url, "r");
      if(!handle) {
        printf("couldn't url_fopen() %s\n", url);
        fclose(outf);
        return 2;
      }

      while(!url_feof(handle)) {
        #if 0  //the fgets method will corrupt binary files if* they contain 0 chars
        url_fgets(buffer, sizeof(buffer), handle);
        fwrite(buffer, 1, strlen(buffer), outf);
        #else
        nread = url_fread(buffer, 1, sizeof(buffer), handle);
        fwrite(buffer, 1, nread, outf);
        #endif
      }

      url_fclose(handle);

      fclose(outf);

    #else
      /* Copy from url with fread */
      outf = fopen(FREADFILE, "wb+");
      if(!outf) {
        perror("couldn't open fread output file\n");
        return 1;
      }

      handle = url_fopen("testfile", "r");
      if(!handle) {
        printf("couldn't url_fopen() testfile\n");
        fclose(outf);
        return 2;
      }

      do {
        nread = url_fread(buffer, 1, sizeof(buffer), handle);
        fwrite(buffer, 1, nread, outf);
      } while(nread);

      url_fclose(handle);

      fclose(outf);
    #endif

  #else //新的下载方式
      CURL *curl_handle;
      const char *url;
      static const char *pagefilename = "./page.out";
      FILE *pagefile;

      if(argc < 2) 
      {
        printf("please input <URL>\n");
        url = "http://www.baidu.com/index.html";
        //url = "https://qdcu01.baidupcs.com/file/96bb1f5e83aa621948f7d7f3b0ff8cf7?bkt=p3-140096bb1f5e83aa621948f7d7f3b0ff8cf7e210d6240000000001e8&fid=557564212-250528-965041864654337&time=1541644892&sign=FDTAXGERLQBHSKW-DCb740ccc5511e5e8fedcff06b081203-ffsWPJRwI8SLYcKH7xEFeIzHgCc%3D&to=65&size=488&sta_dx=488&sta_cs=7&sta_ft=txt&sta_ct=1&sta_mt=1&fm2=MH%2CQingdao%2CAnywhere%2C%2Cguangdong%2Cct&ctime=1541399159&mtime=1541399159&resv0=cdnback&resv1=0&vuk=557564212&iv=0&htype=&newver=1&newfm=1&secfm=1&flow_ver=3&pkey=140096bb1f5e83aa621948f7d7f3b0ff8cf7e210d6240000000001e8&sl=76480590&expires=8h&rt=sh&r=525310594&mlogid=7223300540478406269&vbdid=2678328550&fin=test_update.txt&fn=test_update.txt&rtype=1&dp-logid=7223300540478406269&dp-callid=0.1.1&hps=1&tsl=80&csl=80&csign=gK8PVmsTwsBYTJzeE6BWLbZWL60%3D&so=0&ut=6&uter=4&serv=0&uc=1694538685&ti=37cfb58296b21ea93ad4268cf83c67e37646cebefeffe855&by=themis";
      }
      else
      {

        url = argv[1];        
      }

      printf("argv[1] = %s\n",argv[1]);
      
      curl_global_init(CURL_GLOBAL_ALL);

      /* init the curl session */
      curl_handle = curl_easy_init();

      /* set URL to get here */
      curl_easy_setopt(curl_handle, CURLOPT_URL, url);

      /* Switch on full protocol/debug output while testing */
      curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

      /* disable progress meter, set to 0L to enable and disable debug output */
      curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

      /* send all data to this function  */
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

      /* open the file */
      pagefile = fopen(pagefilename, "wb");
      if(pagefile) {

        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        /* get it! */
        curl_easy_perform(curl_handle);

        /* close the header file */
        fclose(pagefile);
      }

      /* cleanup curl stuff */
      curl_easy_cleanup(curl_handle);

      curl_global_cleanup();


  #endif

  return 0;/* all done */
}
