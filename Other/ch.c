#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
        curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");

        // see RFC6855 IMAP Support for UTF-8
        // imap.gmail.com states UTF8=ACCEPT in CAPABILITY response,
        // so enable it to use UTF-8 in quoted strings.
        // Must come after AUTHENTICATE and before SELECT.
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "ENABLE UTF8=ACCEPT");
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        // SELECT INBOX broken out from URL
        if (res == CURLE_OK) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "SELECT INBOX");
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
        }

        if (res == CURLE_OK) {
            // U+00FC LATIN SMALL LETTER U WITH DIAERESIS is \xC3\xBC in UTF-8
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "UID SEARCH ALL");
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
    return (int) res;
}