
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2020, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

/* <DESC>
 * IMAP example showing how to retreieve e-mails
 * </DESC>
 */

#include <stdio.h>

#include <curl/curl.h>
/* This is a simple example showing how to fetch mail using libcurl's IMAP
 * capabilities.
 *
 * Note that this example requires libcurl 7.30.0 or above.
 */

int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "testhailaude@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "testing321");

    /* This will fetch message 1 from the user's inbox */
    curl_easy_setopt(curl, CURLOPT_URL,
                     "imaps://imap.gmail.com:993/INBOX");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,
                     "SEARCH SUBJECT abc");
    /* Perform the fetch */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* Always cleanup */
    curl_easy_cleanup(curl);
  }

  return (int)res;
}