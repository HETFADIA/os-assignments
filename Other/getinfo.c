#include <stdio.h>
#include <curl/curl.h>
 
/* This is a simple example showing how to obtain information about a mailbox
 * folder using libcurl's IMAP capabilities via the EXAMINE command.
 *
 * Note that this example requires libcurl 7.30.0 or above.
 */
 
int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
 
  curl = curl_easy_init();
  if(curl) {
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
 
    /* This is just the server URL */
    curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");
 
    /* Set the EXAMINE command specifying the mailbox folder */
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "EXAMINE INBOX");
 
    /* Perform the custom request */
    res = curl_easy_perform(curl);
 
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* Always cleanup */
    curl_easy_cleanup(curl);
  }
 
  return (int)res;
}