#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
#include <ctype.h>
#include <stdbool.h>
#define chr char
#define lld int
chr imap[128];
chr smtp[128];
chr user[128];
chr pswd[128];
lld PORT_i = -1;
lld PORT_s = -1;

struct MemoryStruct
{
    chr *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    size_t realsize = size * nmemb;
    lld ptrsize = mem->size + realsize + 1;
    chr *ptr = realloc(mem->memory, mem->size + realsize + 1);
    bool ptrvalid = !ptr;
    if (ptrvalid)
    {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    else
    {
        mem->memory = ptr;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    lld lastindex = mem->size;
    mem->memory[lastindex] = 0;
    printf("The mem memory is %s\n", mem->memory);

    return realsize;
}

lld searchbysubject(chr a[])
{
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;
    char *uid_search_sub = "UID SEARCH SUBJECT \"\"";
    chr *res_s = calloc(strlen(a) + 21 + 1 + 1, sizeof(chr));
    strcat(res_s, uid_search_sub);
    strcat(res_s, a);

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    char *urls = "imaps://imap.gmail.com:993/INBOX";
    curl_easy_setopt(curl_handle, CURLOPT_URL, urls);
    if (strlen(res_s))
    {

        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, res_s);
    }
    else
    {
        printf("UID cant be empty\n");
    }
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    lld val = -1;
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        chr *p = chunk.memory;
        printf("%s", p);
        while (*p)
        {
            bool _isdigit = isdigit(*p);
            if (_isdigit)
            {
                val = strtol(p, &p, 10);
                printf("%d", val);
            }
            else
            {
                ++p;
            }
        }
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return val;
}

void Smail()
{
    lld UID = searchbysubject("IMP");
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chr number[5];
    memset(number, 0, 5);
    sprintf(number, "%d", UID);

    chr *res_s = calloc(strlen(number) + 39, sizeof(chr));
    strcat(res_s, "imaps://imap.gmail.com:993/INBOX/;UID=");
    strcat(res_s, number);

    chunk.memory = malloc(1);
    chunk.size = 0;
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
    if (strlen(pswd) == 0)
    {
        printf("Password cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    }
    if (strlen(res_s))
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, res_s);
    }
    else
    {
        printf("URL cant be empty\n");
        return;
    }
    lld val = -1;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
    {
        val = -1;
        printf("The curl found error");
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        val = 1;
        printf("Executed successfully %s\n", chunk.memory);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

void delete_sub(chr a[])
{
    lld UID = searchbysubject(a);
    if (UID == -1)
    {
        printf("\nNo such subject as %s found in the email\n", a);
        return;
    }
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chr number[5];
    for (lld i = 0; i < 5; i++)
    {
        number[i] = '\0';
    }
    sprintf(number, "%d", UID);

    chr *res_s = calloc(strlen(number) + 11 + 19, sizeof(chr));
    strcat(res_s, "UID STORE ");
    strcat(res_s, number);
    strcat(res_s, " +FLAGS (\\Deleted)");

    if (strlen(res_s) == 0)
    {
        printf("Error as res can not be empty\n");
        return;
    }

    chunk.memory = malloc(1);
    chunk.size = 0;
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
    if (strlen(pswd) == 0)
    {
        printf("Password cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    if (strlen(res_s) == 0)
    {
        printf("URL cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, res_s);
    }
    lld val = -1;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        printf("%s \n Delete done", chunk.memory);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

// chr **payload_text;

chr *payload_text[] = {
    "Subject: SMTP example message",
    "\r\n",
    "message",
    "\r\n",
    NULL};

struct upload_status
{
    lld lines_read;
};
chr *res_s;
chr *body_msg;
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const chr *data;
    if (size * nmemb < 1)
    {
        return 0;
    }
    data = payload_text[upload_ctx->lines_read];

    if (data)
    {
        size_t len = strlen(data);
        if (upload_ctx->lines_read == 0)
        {
            len = strlen(res_s);
            memcpy(ptr, res_s, len);
        }
        else if (strcmp(data, "\r\n") != 0)
        {
            len = strlen(body_msg);
            memcpy(ptr, body_msg, len);
        }
        else
        {
            memcpy(ptr, data, len);
        }
        upload_ctx->lines_read++;
        return len;
    }
    return 0;
}
chr *email_name = NULL;
chr *url_name = NULL;
void send_mail(chr sub[], chr text[])
{
    res_s = calloc(strlen(sub) + 10, sizeof(chr));
    strcat(res_s, "Subject: ");
    strcat(res_s, sub);
    printf("res_s is %s\n", res_s);
    body_msg = calloc(strlen(text) + 1, sizeof(chr));
    struct upload_status upload_ctx;
    upload_ctx.lines_read = 0;
    for (lld i = 0; i < strlen(text); ++i)
    {
        body_msg[i] = text[i];
    }

    CURL *curl;
    CURLcode res;
    res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com");
        printf("user is %s\n", user);
        recipients = curl_slist_append(recipients, "cluetest29@gmail.com");
        curl_easy_setopt(curl, CURLOPT_USERNAME, "cluetest29@gmail.com");
        // recipients = curl_slist_append(recipients, "testhailaude@gmail.com");
        // curl_easy_setopt(curl, CURLOPT_USERNAME, "testhailaude@gmail.com");
        if (strlen(pswd) > 0)
        {
            curl_easy_setopt(curl, CURLOPT_PASSWORD, pswd);
        }
        else
        {
            printf("Enter non empty password in configure.txt\n");
        }

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);

        lld val = -1;
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            val = -1;
            printf("The curl found error");
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
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
    lld buflen = 128;
    chr buf[buflen];

    f = fopen(argv[2], "r");
    memset(buf, 0, sizeof(buf));
    fgets(buf, buflen, f);
    // imap
    strcpy(imap, buf);
    if (strcmp(imap, "") == 0)
    {
        printf("Please enter your imap server\n");
        exit(-1);
    }
    printf("imap is %s\n", imap);
    memset(buf, 0, sizeof(buf));
    fgets(buf, buflen, f);
    // smtp
    strcpy(smtp, buf);
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
    memset(buf, 0, sizeof(buf));
    fgets(buf, buflen, f);
    // imap port num
    PORT_i = atoi(buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, buflen, f);
    // smtp port num
    PORT_s = atoi(buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, buflen, f);
    if (PORT_i == -1 || PORT_s == -1)
    {
        printf("\nPort number error\n");
        printf("\nPORT not supplied\n");
        exit(-1);
    }
    // username
    strcpy(user, buf);
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
    memset(buf, 0, sizeof(buf));
    fgets(buf, buflen, f);
    // password
    strcpy(pswd, buf);
    if (strcmp(pswd, "") == 0)
    {
        printf("Please enter your password\n");
        exit(-1);
    }
    fclose(f);

    // Smail();

    printf("\n------------------------------\n");
    printf("%s", pswd);
    delete_sub("security alert");
    send_mail("This is subject", "This is body");
}