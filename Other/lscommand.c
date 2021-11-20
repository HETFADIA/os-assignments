#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

/* This is a simple example showing how to list the folders within an IMAP
 * mailbox.
 *
 * Note that this example requires libcurl 7.30.0 or above.
 */

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
 
  curl = curl_easy_init();
  if(curl) {
    struct string s;
    init_string(&s);
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
 
    /* This will list the folders within the user's mailbox. If you want to
     * list the folders within a specific folder, for example the inbox, then
     * specify the folder as a path in the URL such as /INBOX */
    curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
 
    /* Perform the list */
    res = curl_easy_perform(curl);
 
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    char ss[] = {'"','/','"'};
    printf("%s\n\n",ss);
    printf("%s\n", s.ptr);
    free(s.ptr);

    /* Always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}