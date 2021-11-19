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

char imap[100], smtp[100], user[100], pswd[100];
int PORT_i = -1, PORT_s = -1;

struct MemoryStruct
{
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
    {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int searchbysubject(char a[])
{
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    char *res_s = calloc(strlen(a) + 21 + 2, sizeof(char));
    strcat(res_s, "UID SEARCH SUBJECT \"");
    strcat(res_s, a);
    strcat(res_s, "\"");

    // printf("%s", res_s);

    chunk.memory = malloc(1);
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    curl_easy_setopt(curl_handle, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, res_s);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    int val = -1;
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        char *p = chunk.memory;
        while (*p)
        {
            if (isdigit(*p))
            {
                val = strtol(p, &p, 10);
                printf("%d", val);
            }
            else
            {
                p++;
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
    int UID = searchbysubject("IMP");
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    char number[5];
    memset(number, 0, 5);
    sprintf(number, "%d", UID);

    char *res_s = calloc(strlen(number) + 39, sizeof(char));
    strcat(res_s, "imaps://imap.gmail.com:993/INBOX/;UID=");
    strcat(res_s, number);

    chunk.memory = malloc(1);
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    curl_easy_setopt(curl_handle, CURLOPT_URL, res_s);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    int val;
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        printf("%s YO", chunk.memory);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

void delete_sub(char a[])
{
    int UID = searchbysubject(a);
    if (UID == -1)
    {
        printf("\nNo such subject as %s found in the email\n", a);
        return;
    }
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    char number[5];
    memset(number, 0, 5);
    sprintf(number, "%d", UID);

    char *res_s = calloc(strlen(number) + 11 + 19, sizeof(char));
    strcat(res_s, "UID STORE ");
    strcat(res_s, number);
    strcat(res_s, " +FLAGS (\\Deleted)");

    printf("%s", res_s);

    chunk.memory = malloc(1);
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    curl_easy_setopt(curl_handle, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, res_s);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    int val;
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        printf("%s \n Delete done", chunk.memory);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

// char **payload_text;

char *payload_text[] = {
    "Subject: SMTP example message",
    "\r\n",
    "Bhag bsdk",
    "\r\n",
    NULL};

struct upload_status
{
    int lines_read;
};
char *res_s;
char *body_msg;
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    if (size * nmemb < 1)
        return 0;
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

void send_mail(char sub[], char text[])
{
    res_s = calloc(strlen(sub) + 10, sizeof(char));
    strcat(res_s, "Subject: ");
    strcat(res_s, sub);
    printf("res_s is %s\n", res_s);
    body_msg = calloc(strlen(text) + 1, sizeof(char));
    for (int i = 0; i < strlen(text); ++i)
    {
        body_msg[i] = text[i];
    }
    // char *payload_text_1[] = {
    // res_s,
    // "\r\n",
    // text,
    // "\r\n",
    // NULL
    // };
    // payload_text = payload_text_1;
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com");
        printf("%s", user);
        recipients = curl_slist_append(recipients, "testhailaude@gmail.com");
        curl_easy_setopt(curl, CURLOPT_USERNAME, "testhailaude@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pswd);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}

// static int SS_getattr(const char *path, struct stat *st){
//     st->st_uid = getuid();
// 	st->st_gid = getgid();
// 	st->st_atime = time(NULL);
// 	st->st_mtime = time(NULL);

//     path++; // removing "/" from the path
// 	int isFile = 0, isFldr = 0;
// 	for(int i = 0; i <= var_file_len; i++){
// 		if (strcmp(path, SS_filelist[i]) == 0){
// 			isFile = 1;
//             break;
//         }
//     }

//     for(int i = 0; i <= var_fldr_len; i++){
// 		if (strcmp(path, SS_fldrlist[i]) == 0){
// 			isFldr = 1;
//             break;
//         }
//     }

//     if(strcmp( path, "/" ) == 0 || isFldr){
//         st->st_mode = S_IFDIR | 0755;
// 		st->st_nlink = 2;
//     }
//     else if(isFile){
//         st->st_mode = S_IFREG | 0644;
// 		st->st_nlink = 1;
// 		st->st_size = 1024;
//     }
//     else{
//         return -ENOENT;
//     }
//     return 0;
// }

// static struct fuse_operations operations = {
//     .getattr	= do_getattr,
//     .readdir	= do_readdir,
//     .read		= do_read,
//     .mkdir		= do_mkdir,
//     .mknod		= do_mknod,
//     .write		= do_write,
// };

int main(int argc, char **argv)
{
    /*
    gcc send_delete.c `pkg-config fuse --cflags --libs` -lcurl
    ./a.out  store configure.txt

    */
    if (argc != 3)
    {
        printf("./a.out [File mount path] [File connection info]");
        exit(-1);
    }
    // argv[1] mount file
    // argv[2] file
    FILE *f;
    char buf[100];

    f = fopen(argv[2], "r");
    memset(buf, 0, sizeof(buf));
    fgets(buf, 100, f);
    // imap
    strcpy(imap, buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, 100, f);
    // smtp
    strcpy(smtp, buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, 100, f);
    // imap port num
    PORT_i = atoi(buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, 100, f);
    // smtp port num
    PORT_s = atoi(buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, 100, f);
    if (PORT_i == -1 || PORT_s == -1)
    {
        printf("\nPort number error\n");
        printf("\nPORT not supplied\n");
        exit(-1);
    }
    // username
    strcpy(user, buf);
    memset(buf, 0, sizeof(buf));
    fgets(buf, 100, f);
    // password
    strcpy(pswd, buf);
    fclose(f);

    // Smail();

    printf("\n------------------------------\n");
    printf("%s", pswd);
    // delete_sub("1");
    send_mail("hello", "Starboy\n I want a ");

    // return fuse_main(argc-1, argv, &operations, NULL);
}