#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
 
/* This is a simple example showing how to fetch mail using libcurl's IMAP
 * capabilities.
 *
 * Note that this example requires libcurl 7.30.0 or above.
 */
 

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}


int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;

    struct MemoryStruct chunk;
 
  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */
 
  curl = curl_easy_init();
  if(curl) {
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "testhailaude@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "testing321");
 
    /* This will fetch message 1 from the user's inbox */
    curl_easy_setopt(curl, CURLOPT_URL,
                     "imaps://imap.gmail.com:993/INBOX");
                    //  "imaps://imap.gmail.com:993/INBOX/;UID=1");

    // curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,
    //                  "SELECT \"INBOX\"");

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,
                     "UID FETCH 4 BODY[HEADER.FIELDS (TEXT)]");

                       /* send all data to this function  */
 
    /* Perform the fetch */
    res = curl_easy_perform(curl);
 
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    else{
        printf("%sboo\n", chunk.memory);
    }
 
    /* Always cleanup */
    curl_easy_cleanup(curl);
  }

 
  return (int)res;
}