#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

/* This is a simple example showing how to list the folders within an IMAP
 * mailbox.
 *
 * Note that this example requires libcurl 7.30.0 or above.
 */

struct string_ptr {
  char *ptr;
  size_t len;
};

void init_string_ptr(struct string_ptr *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string_ptr *s)
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

int is_dir( const char *path )
{
	// path++; // Eliminating "/" in the path
	
	// for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
	// 	if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
	// 		return 1;
	
	// return 0;

	int len=(int)strlen(path)+6;
	char pattern[len];

	pattern[0] = '"'; 
	pattern[1] = '/';
	pattern[2] = '"';
	pattern[3] = ' ';
	pattern[4] = '"';

	char *t;
	int first_slash = 1;
	int index = 5;
	for(t = path; *t != '\0'; t++){
		if(*t == '/' && first_slash == 1){
			first_slash = 0;
			continue;
		}
		pattern[index] = *t; index++;
	}

	pattern[index] = '"';
	index++;
	pattern[index] = '\0';

	CURL *curl;
    curl = curl_easy_init();
	CURLcode res = CURLE_OK;
	struct string_ptr s;
    if(curl) {
    	init_string_ptr(&s);
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
		
		/* Always cleanup */
		curl_easy_cleanup(curl);
    }

	for(int i = 0; i < (int)strlen(s.ptr); i++){
		int curr = i;
		int matched_len = 0;
		for(t = pattern; *t != '\0'; t++){
			if(s.ptr[curr] != *t){
				break;
			}
			curr++;
			matched_len++;
			if(matched_len == len - 1) return 1;
		}
	}
    return 0;
}

int main(void)
{
  printf("%d\n\n",is_dir("/[Gmail]/Spam"));
  return 0;
}