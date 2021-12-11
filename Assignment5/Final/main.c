#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
#include <ctype.h>
#define chr char
#define lld int
chr imap[128];
chr smtp[128];
chr user[128];
chr password[128];
lld firstport = -1;
lld secondport = -1;

struct __dataStruct
{
    chr *__data;
    size_t size;
};

static size_t Write__dataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    struct __dataStruct *_data_ = (struct __dataStruct *)userp;
    size_t correctsize = size * nmemb;
    lld pointersize = _data_->size + correctsize + 1;
    chr *pointer = realloc(_data_->__data, _data_->size + correctsize + 1);
    bool pointervalid = !pointer;
    if (pointervalid)
    {
        printf("not enough __data (realloc returned NULL)\n");
        return 0;
    }
    else
    {
        _data_->__data = pointer;
    }
    memcpy(&(_data_->__data[_data_->size]), contents, correctsize);
    _data_->size += correctsize;
    lld lastindex = _data_->size;
    _data_->__data[lastindex] = 0;
    printf("The _data_ __data is %s\n", _data_->__data);

    return correctsize;
}

lld searchbysubject(chr a[])
{
    CURL *curl_handle;
    CURLcode _result1;
    struct __dataStruct partial;
    partial.__data = malloc(1);
    partial.size = 0;
    char *uid_search_sub = "UID SEARCH SUBJECT \"\"";
    chr *__result = calloc(strlen(a) + 21 + 1 + 1, sizeof(chr));
    strcat(__result, uid_search_sub);
    strcat(__result, a);

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, password);
    char *urls = "imaps://imap.gmail.com:993/INBOX";
    curl_easy_setopt(curl_handle, CURLOPT_URL, urls);
    if (strlen(__result))
    {

        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, __result);
    }
    else
    {
        printf("UID cant be empty\n");
    }
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Write__dataCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&partial);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    _result1 = curl_easy_perform(curl_handle);
    lld val = -1;
    if (_result1 != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(_result1));
    }
    else
    {
        chr *dataptr = partial.__data;
        printf("%s", dataptr);
        while (*dataptr)
        {
            bool _isdigit = isdigit(*dataptr);
            if (_isdigit)
            {
                val = strtol(dataptr, &dataptr, 10);
                printf("%d", val);
            }
            else
            {
                ++dataptr;
            }
        }
    }
    curl_easy_cleanup(curl_handle);
    free(partial.__data);
    curl_global_cleanup();

    return val;
}

void init()
{
    lld UID = searchbysubject("IMP");
    CURL *curl_handle;
    CURLcode _result1;
    struct __dataStruct partial;

    chr number[5];
    memset(number, 0, 5);
    sprintf(number, "%d", UID);

    chr *__result = calloc(strlen(number) + 39, sizeof(chr));
    strcat(__result, "imaps://imap.gmail.com:993/INBOX/;UID=");
    strcat(__result, number);

    partial.__data = malloc(1);
    partial.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (strlen(user) == 0)
    {
        printf("User cant be empty\n");
        return;
    }
    else
    {

        curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    }
    if (strlen(password) == 0)
    {
        printf("Password cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, password);
    }
    if (strlen(__result))
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, __result);
    }
    else
    {
        printf("URL cant be empty\n");
        return;
    }
    lld val = -1;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Write__dataCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&partial);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    _result1 = curl_easy_perform(curl_handle);
    if (_result1 != CURLE_OK)
    {
        val = -1;
        printf("The curl found error");
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(_result1));
    }
    else
    {
        val = 1;
        printf("Executed successfully %s\n", partial.__data);
    }
    curl_easy_cleanup(curl_handle);
    free(partial.__data);
    curl_global_cleanup();
}

void delete_mail_by_subject(chr a[])
{
    lld UID = searchbysubject(a);
    if (UID == -1)
    {
        printf("\nNo such subject as %s found in the email\n", a);
        return;
    }
    CURL *curl_handle;
    CURLcode _result1;
    struct __dataStruct partial;

    chr number[5];
    for (lld i = 0; i < 5; i++)
    {
        number[i] = '\0';
    }
    sprintf(number, "%d", UID);

    chr *__result = calloc(strlen(number) + 11 + 19, sizeof(chr));
    strcat(__result, "UID STORE ");
    strcat(__result, number);
    strcat(__result, " +FLAGS (\\Deleted)");

    if (strlen(__result) == 0)
    {
        printf("Error as _result1 can not be empty\n");
        return;
    }

    partial.__data = malloc(1);
    partial.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (strlen(user) == 0)
    {
        printf("User cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    }
    if (strlen(password) == 0)
    {
        printf("Password cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, password);
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    if (strlen(__result) == 0)
    {
        printf("URL cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, __result);
    }
    lld val = -1;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Write__dataCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&partial);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    _result1 = curl_easy_perform(curl_handle);
    if (_result1 != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(_result1));
    }
    else
    {
        printf("%s \n Delete done", partial.__data);
    }
    curl_easy_cleanup(curl_handle);
    free(partial.__data);
    curl_global_cleanup();
}

// chr **information_text;

chr *information_text[] = {
    "Subject: SMTP example message",
    "\r\n",
    "message",
    "\r\n",
    NULL};

struct upload_status
{
    lld lines_read;
};
chr *__result;
chr *body_msg;
static size_t information_source(void *pointer, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const chr *data;
    if (size * nmemb < 1)
    {
        return 0;
    }
    data = information_text[upload_ctx->lines_read];

    if (data)
    {
        size_t len = strlen(data);
        if (upload_ctx->lines_read == 0)
        {
            len = strlen(__result);
            memcpy(pointer, __result, len);
        }
        else if (strcmp(data, "\r\n") != 0)
        {
            len = strlen(body_msg);
            memcpy(pointer, body_msg, len);
        }
        else
        {
            memcpy(pointer, data, len);
        }
        upload_ctx->lines_read++;
        return len;
    }
    return 0;
}
chr *email_name = NULL;
chr *url_name = NULL;
void sendemail(chr sub[], chr text[])
{
    __result = calloc(strlen(sub) + 10, sizeof(chr));
    strcat(__result, "Subject: ");
    strcat(__result, sub);
    printf("__result is %s\n", __result);
    body_msg = calloc(strlen(text) + 1, sizeof(chr));
    struct upload_status upload_ctx;
    upload_ctx.lines_read = 0;
    for (lld i = 0; i < strlen(text); ++i)
    {
        body_msg[i] = text[i];
    }

    CURL *curl;
    CURLcode _result1;
    _result1 = CURLE_OK;
    struct curl_slist *recipients = NULL;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com");
        printf("user is %s\n", user);
        recipients = curl_slist_append(recipients, "cluetest29@gmail.com");
        curl_easy_setopt(curl, CURLOPT_USERNAME, "cluetest29@gmail.com");
        if (strlen(password) > 0)
        {
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        }
        else
        {
            printf("Enter non empty password in configure.txt\n");
        }

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, information_source);

        lld val = -1;
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        _result1 = curl_easy_perform(curl);
        if (_result1 != CURLE_OK)
        {
            val = -1;
            printf("The curl found error");
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(_result1));
        }
        else
        {
            val = 1;
            printf("Executed successfully \n");
        }
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}
lld main(lld argc, chr **argv)
{
    /*
    gcc main.c `pkg-config fuse --cflags --libs` -lcurl
    ./a.out  store configure.txt
    */
    if (argc != 3)
    {
        printf("./a.out [File mount path] [File connection info]\n");
        printf("An example of this can be seen below:\n");
        printf("gcc send_delete.c `pkg-config fuse --cflags --libs` -lcurl\n");
        printf("./a.out  store configure.txt\n");
        exit(-1);
    }
    FILE *f;
    lld bufferlen = 128;
    chr buffer[bufferlen];

    f = fopen(argv[2], "r");
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, bufferlen, f);
    // imap
    strcpy(imap, buffer);
    if (strcmp(imap, "") == 0)
    {
        printf("Please enter your imap server\n");
        exit(-1);
    }
    printf("imap is %s\n", imap);
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, bufferlen, f);
    // smtp
    strcpy(smtp, buffer);
    if (strcmp(smtp, "") == 0)
    {
        printf("Please enter your smtp server\n");
        exit(-1);
    }
    else
    {
        lld length = strlen(smtp);
        url_name = (chr *)malloc(length + 1);
        strcpy(url_name, smtp);
    }
    printf("smtp is %s", smtp);
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, bufferlen, f);
    // imap port num
    firstport = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, bufferlen, f);
    // smtp port num
    secondport = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, bufferlen, f);
    if (firstport == -1 || secondport == -1)
    {
        printf("\nPort number error\n");
        printf("\nPORT not supplied\n");
        exit(-1);
    }
    // username
    strcpy(user, buffer);
    if (strcmp(user, "") == 0)
    {
        printf("Please enter your non empty username\n");
        exit(-1);
    }
    else
    {
        email_name = (chr *)malloc(strlen(user) + 1);
        strcpy(email_name, user);
        printf("The name of the email is %s\n", email_name);
    }
    printf("user is %s\n", user);
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, bufferlen, f);
    // password
    strcpy(password, buffer);
    if (strcmp(password, "") == 0)
    {
        printf("Please enter your password\n");
        exit(-1);
    }
    fclose(f);

    printf("%s", password);
    // delete_mail_by_subject("security alert");
    sendemail("This is subject", "This is body");
}