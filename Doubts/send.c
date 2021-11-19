#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

/*
 * For an SMTP example using the multi interface please see smtp-multi.c.
 */

/* The libcurl options want plain addresses, the viewable headers in the mail
 * can very well get a full name as well.
 */
// #define FROM_ADDR    "<testhailaude@gmail.com>"
// #define TO_ADDR      "<testhailaude@gmail.com>"
// #define CC_ADDR      "<testhailaude@gmail.com>"

// #define FROM_MAIL "Madarchod " FROM_ADDR
// #define TO_MAIL   "Behenchod " TO_ADDR
// #define CC_MAIL   "John CC Smith " CC_ADDR

static const char *payload_text[] = {
    // "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
    // // "To: " TO_MAIL "\r\n",
    // "From: " FROM_MAIL "\r\n",
    // "Cc: " CC_MAIL "\r\n",
    "Subject: SMTP example message\r\n",
    "\r\n", /* empty line to divide headers from body, see RFC5322 */
    "Bhag bsdk\r\n",
    "\r\n",
    "Chappal se maroonga\r\n",
    NULL};

struct upload_status
{
    int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
        return 0;
    data = payload_text[upload_ctx->lines_read];
    if (data)
    {
        size_t len = strlen(data);
        memcpy(ptr, data, len);
        upload_ctx->lines_read++;
        return len;
    }
    return 0;
}

int main(void)
{
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com");
        recipients = curl_slist_append(recipients, "testhailaude@gmail.com");
        curl_easy_setopt(curl, CURLOPT_USERNAME, "testhailaude@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "testing321");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    return (int)res;
}