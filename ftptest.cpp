#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#define LOCAL_FILE      "/tmp/uploadthis.txt"
#define UPLOAD_FILE_AS  "plate_0.png"
#define REMOTE_URL      "ftp://184.173.179.109/plate_generic.png"
 static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  curl_off_t nread;
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */ 
  size_t retcode = fread(ptr, size, nmemb, stream);
 
  nread = (curl_off_t)retcode;
 
  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
  return retcode;
}
 
 
int main(void)
{
  CURL *curl;
  CURLcode res;
  FILE *hd_src;
  struct stat file_info;
  curl_off_t fsize;
  struct curl_slist *headerlist = NULL;
  static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
  static const char buf_2 [] = "RNTO plate_newplate.png" ;
  hd_src = fopen("images/plate_0.png","rb");
  curl = curl_easy_init();
  if(curl) {
  

    headerlist = curl_slist_append(headerlist, buf_1);
    headerlist = curl_slist_append(headerlist, buf_2);


// ftp://ttcore:infiniti@184.173.179.109  
//    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    printf("curl is active\n");
     curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    curl_easy_setopt(curl, CURLOPT_URL, REMOTE_URL);
    curl_easy_setopt(curl, CURLOPT_USERNAME, "ttcore");
    curl_easy_setopt(curl, CURLOPT_PASSWORD , "infiniti");
  //  curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
 
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
 
    /* Now run off and do what you've been told! */ 
   res =  curl_easy_perform(curl);
    /* Check for errors */ 
 
    /* clean up the FTP commands list */ 
    //curl_slist_free_all(headerlist);
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
 
  curl_global_cleanup();
  return 0;
}

