#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
 
/* This is a simple example showing how to send mail using libcurl's IMAP
 * capabilities.
 *
 * Note that this example requires libcurl 7.30.0 or above.
 */
 
#define FROM    "<2019csb1067@iitrpr.ac.in>"
#define TO      "<2019csb1070@iitrpr.ac.in>"
#define CC      "<2019csb1077@iitrpr.ac.in>"
 
static const char *payload_text =
  "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n"
  "To: " TO "\r\n"
  "From: " FROM "(Example User)\r\n"
  "Cc: " CC "(Another example User)\r\n"
  "Message-ID: "
  "<dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>\r\n"
  "Subject: IMAP example message\r\n"
  "\r\n" /* empty line to divide headers from body, see RFC5322 */
  "The body of the message starts here.\r\n"
  "\r\n"
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n"
  "Check RFC5322.\r\n";
 
struct upload_status {
  size_t bytes_read;
};
 
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = &payload_text[upload_ctx->bytes_read];
 
  if(*data) {
    size_t len = strlen(data);
    if(room < len)
      len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;
 
    return len;
  }
 
  return 0;
}
 
int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
 
  curl = curl_easy_init();
  if(curl) {
    long infilesize;
    struct upload_status upload_ctx = { 0 };
 
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
 
    /* This will create a new message 100. Note that you should perform an
     * EXAMINE command to obtain the UID of the next message to create and a
     * SELECT to ensure you are creating the message in the OUTBOX. */
    curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com/INBOX");
 
    /* In this case, we are using a callback function to specify the data. You
     * could just use the CURLOPT_READDATA option to specify a FILE pointer to
     * read from. */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    infilesize = strlen(payload_text);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, infilesize);
 
    /* Perform the append */
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