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
#define lld int
#define chr char
chr imap[128];
chr smtp[128];
chr user[128];
chr pswd[128];
lld PORT_i = -1;
lld PORT_s = -1;
char *strip(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            break;
        }
        i++;
    }
    return str;
}
char *split(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == ':')
        {
            str[i] = '\0';
            break;
        }
        i++;
    }
    return str;
}

struct MemoryStruct
{
    size_t size;

    chr *memory;
};

chr *l_t(chr *a)
{
    while (!isalnum(*a))
    {
        a++;
    }
    return a;
}
chr *r_t(chr *a)
{
    chr *rev = a + strlen(a);
    while (!isalnum(*--rev))
    {
        // empty loop
    }
    *(rev + 1) = 0;
    return a;
}
chr *trim(chr *a)
{
    lld isEmpty = 1;
    lld strlena = strlen(a);

    for (lld i = 0; i < strlena; i++)
    {
        bool _isalnum = isalnum(a[i]);
        if (isalnum(a[i]))
        {
            isEmpty = 0;
        }
    }
    bool _isEmpty = !isEmpty;
    if (_isEmpty)
    {

        return r_t(l_t(a));
    }

    *a = 0;
    return a;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize;
    realsize = size;
    realsize *= nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    chr *ptr = realloc(mem->memory, mem->size + realsize + 1);
    bool _ptr = !ptr;
    if (_ptr)
    {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    else
    {
        mem->memory = ptr;
        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->memory[mem->size] = 0;
    }
    mem->size += realsize;

    return realsize;
}

lld searchbysubject(chr a[])
{
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;
    char uid_search_sub = "UID SEARCH SUBJECT \"";
    chr *res_s = calloc(strlen(a) + 21 + 1 + 1, sizeof(chr));
    strcat(res_s, uid_search_sub);
    strcat(res_s, a);
    strcat(res_s, "\"");

    // printf("%s", res_s);

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    char *urls = "imaps://imap.gmail.com:993/INBOX";
    curl_easy_setopt(curl_handle, CURLOPT_URL, urls);
    if (strlen(res_s) == 0)
    {
        printf("UID cant be empty\n");
    }
    else
    {

        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, res_s);
    }
    lld val = 0;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, val);
    res = curl_easy_perform(curl_handle);
    val = -1;
    bool _res = res != CURLE_OK;
    if (_res)
    {
        printf("The curl did not get executed due to some error\n");
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        chr *p = chunk.memory;
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

    if (val == -1)
    {
        printf("Error\n");
        exit(-1);
    }

    return val;
}
lld fldr_i = -1;
lld files_r_i = -1;
lld files_c = -1;
chr Folder[1024][300];
lld curr_ind[1024];
chr Files_in_d[1024][1024][300];
chr Files_in_r[1024][300];
chr sub_in_r[1024][300];
chr sub_in_d[1024][1024][300];
chr all_files[10000][300];
void initialize()
{
    memset(Folder, 0, sizeof(Folder));
    memset(Files_in_d, 0, sizeof(Files_in_d));
    memset(Files_in_d, 0, sizeof(Files_in_d));
    memset(Files_in_r, 0, sizeof(Files_in_r));
    memset(sub_in_r, 0, sizeof(sub_in_r));
    memset(sub_in_d, 0, sizeof(sub_in_d));
    memset(all_files, 0, sizeof(all_files));
}
lld find_dir(chr a[])
{
    for (lld i = 0; i <= fldr_i; ++i)
    {
        if (strcmp(Folder[i], a) == 0)
        {
            return i;
        }
    }
    return -1;
}

lld file_in_dir(chr a[], lld dir_loc)
{
    for (lld i = 0; i < curr_ind[dir_loc]; ++i)
    {
        if (strcmp(Files_in_d[dir_loc][i], a) == 0)
        {
            return i;
        }
    }
    return -1;
}

void Smail()
{
    lld UID = searchbysubject("IMP");
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chr number[5];
    for (lld i = 0; i < 5; ++i)
    {
        number[i] = 0;
    }
    chunk.memory = malloc(1);
    chunk.size = 0;
    sprintf(number, "%d", UID);

    chr *res_s = calloc(strlen(number) + 39, sizeof(chr));
    strcat(res_s, "imaps://imap.gmail.com:993/INBOX/;UID=");
    strcat(res_s, number);

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
    if (strlen(res_s) == 0)
    {
        printf("URL cant be empty\n");
        return;
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, res_s);
    }
    lld val;
    bool flag = 0;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    flag = res != CURLE_OK;
    if (flag)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {

        chr *p = chunk.memory;
        chr match_s[] = "Content-Type: text/plain; charset=\"UTF-8\"";
        chr match_s2[] = "Subject: IMP";
        // strcpy(match_s2, "IMP");
        chr *st = strstr(chunk.memory, match_s);
        lld ind;
        bool flag = st != NULL;
        if (flag)
        {
            ind = st - p + strlen(match_s);
        }
        else
        {

            ind = strstr(chunk.memory, match_s2) - p + strlen(match_s2);
        }
        chr line[1024];
        for (lld i = 0; i < 1024; ++i)
        {
            line[i] = 0;
        }
        memset(line, 0, 1024);
        lld k = 0;

        for (lld i = ind; chunk.memory[i] != '-' && i < strlen(chunk.memory); ++i)
        {
            bool a1 = chunk.memory[i] == '/';
            bool a2 = chunk.memory[i] == '.';
            bool a3 = chunk.memory[i] == '=';
            bool a4 = isalnum(chunk.memory[i]);
            if (a1 || a2 || a3 || a4)
            {
                line[k++] = chunk.memory[i];
            }
            else
            {
                k = 0;
                if (strlen(line))
                {
                    printf("%s\n", line);
                    lld isFile = 0;
                    lld strlenline = strlen(line);
                    for (lld i = 0; i < strlenline; i++)
                    {
                        if (line[i] == '=')
                        {
                            isFile = 1;
                            break;
                        }
                    }
                    if (isFile)
                    {
                        lld curr = -1;
                        int strlenline = strlen(line);
                        for (lld i = 0; i < strlenline; ++i)
                        {
                            if (line[i] == '=')
                            {
                                curr = i;
                                break;
                            }
                        }
                        if (curr == -1)
                        {
                            printf("corrupted file\n");
                            exit(-1);
                        }
                        chr *x1 = line;
                        chr *x2 = line + curr + 1;
                        line[curr] = 0;
                        lld cnt = 0, cntpnt = -1;
                        lld strlenx1 = strlen(x1);
                        for (lld i = 0; i < strlenx1; ++i)
                        {
                            if (x1[i] == '/')
                            {
                                ++cnt;
                                if (cnt == 2)
                                {
                                    cntpnt = i;
                                    break;
                                }
                            }
                        }
                        if (cnt == 2)
                        {
                            chr *y1 = x1;
                            y1++;
                            chr *y2 = x1 + cntpnt + 1;
                            x1[cntpnt] = 0;
                            lld dir_loc;
                            dir_loc = find_dir(y1);
                            if (dir_loc == -1)
                            {
                                printf("Directory not found\n");
                                exit(-1);
                            }
                            else
                            {

                                strcpy(Files_in_d[dir_loc][curr_ind[dir_loc]], y2);
                                strcpy(sub_in_d[dir_loc][curr_ind[dir_loc]], x2);
                                ++files_c;
                                strcpy(all_files[files_c], y2);
                                ++curr_ind[dir_loc];
                                printf("Files dir %s %s %s\n", y1, y2, x2);
                            }
                        }
                        else
                        {
                            ++files_r_i;
                            x1 = x1 + 1;
                            strcpy(Files_in_r[files_r_i], x1);
                            strcpy(sub_in_r[files_r_i], x2);
                            ++files_c;
                            strcpy(all_files[files_c], x1);
                            printf("File root %s %s\n", x1, x2);
                        }
                    }
                    else
                    {
                        chr *ch = line;
                        ch++; // remove '/' from the path
                        fldr_i++;
                        strcpy(Folder[fldr_i], ch);
                        printf("Folder %s\n", ch);
                    }
                    memset(line, 0, 1024);
                    for (lld i = 0; i < 1024; ++i)
                    {
                        line[i] = 0;
                    }
                }
            }
        }
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

chr _BODY[4096];
void get_body(chr read_sub[])
{
    lld UID = searchbysubject(read_sub);
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chr number[5];
    memset(number, 0, 5);
    for (lld i = 0; i < 5; ++i)
    {
        number[i] = 0;
    }

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
        printf("The username can't be empty\n");
    }
    else
    {

        curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    }
    if (strlen(pswd) == 0)
    {
        printf("The password can't be empty\n");
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    }
    if (strlen(res_s) == 0)
    {
        printf("The url can't be empty\n");
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, res_s);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    }
    int length = 1024;
    chr *mail_body_text = calloc(length, sizeof(chr));
    lld val = 0;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
    {
        printf("Error occured in curl\n");
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        chr *p = chunk.memory;
        chr *match_s = calloc(11 + strlen(read_sub), sizeof(chr));
        strcat(match_s, "Subject: ");
        strcat(match_s, read_sub);
        chr *st = strstr(chunk.memory, match_s);
        lld ind = st - p;
        ind += strlen(match_s);
        chr line[1024];
        memset(line, 0, 1024);
        for (lld i = 0; i < 1024; ++i)
        {
            line[i] = 0;
        }
        lld k = 0;
        int strlen_chunk = strlen(chunk.memory);
        for (lld i = ind; i < strlen_chunk; ++i)
        {
            bool end_line = chunk.memory[i] == '\n';
            if (end_line)
            {
                line[k++] = chunk.memory[i];
            }
            else
            {
                int strlenline = strlen(line);
                if (strlenline)
                {
                    strcat(mail_body_text, line);
                    strcat(mail_body_text, "\n");

                    for (lld i = 0; i < 1024; ++i)
                    {
                        line[i] = 0;
                    }
                }
                k = 0;
            }
        }
        printf("The body is %s ", mail_body_text);
        chr *trimmed_text = trim(mail_body_text);
        memset(_BODY, 0, 1024);
        for (lld i = 0; i < 1024; ++i)
        {
            _BODY[i] = 0;
        }
        strcpy(_BODY, trimmed_text);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

void delete_sub(chr a[])
{
    lld UID = searchbysubject(a);
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chr number[5];
    memset(number, 0, 5);
    for (lld i = 0; i < 5; ++i)
    {
        number[i] = 0;
    }

    sprintf(number, "%d", UID);

    chr *res_s = calloc(strlen(number) + 1 + strlen("UID STORE ") + 19, sizeof(chr));
    char uid_st = "UID STORE ";
    strcat(res_s, uid_st);
    strcat(res_s, number);
    strcat(res_s, " +FLAGS (\\Deleted)");

    if (strlen(res_s))
    {
        printf("The res= %s", res_s);
    }

    chunk.memory = malloc(1);
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (strlen(user) == 0)
    {
        printf("The username can't be empty\n");
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    }
    if (strlen(pswd) == 0)
    {
        printf("The password can't be empty\n");
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pswd);
    }
    char *inbox_url = "imaps://imap.gmail.com:993/INBOX";
    curl_easy_setopt(curl_handle, CURLOPT_URL, inbox_url);
    if (strlen(res_s) == 0)
    {
        printf("The url can't be empty\n");
    }
    else
    {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, res_s);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    }
    lld val = 0;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
    {
        printf("Error occured in curl file\n");
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

chr text[1024];
chr sub_[30];

struct upload_status
{
    lld lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    chr data[1025];
    for (lld i = 0; i < 1025; ++i)
    {
        data[i] = 0;
    }

    if (size * nmemb < 1 || size == 0 || nmemb == 0)
    {
        return 0;
    }
    int value = upload_ctx->lines_read;
    if (value == 0)
    {
        // subject
        strcpy(data, sub_);
    }
    else if (value == 1)
    {
        strcpy(data, "\r\n");
    }
    else if (value == 3)
    {
        strcpy(data, "\r\n");
    }
    else if (value == 4)
    {
        return 0;
    }
    else
    {
        // text
        strcpy(data, text);
    }
    if (data)
    {
        size_t len = strlen(data);
        memcpy(ptr, data, len);
        ++upload_ctx->lines_read;
        return len;
    }
    return 0;
}
char * email_name=NULL;
char * url_name=NULL;
void send_mail(chr SS_sub[], chr SS_text[])
{
    CURL *curl;
    CURLcode res;
    res = CURLE_OK;
    chr *res_s = calloc(strlen(SS_sub) + 10, sizeof(chr));
    chr start = "Subject: ";
    strcat(res_s, start);
    strcat(res_s, SS_sub);
    strcpy(sub_, res_s);
    strcpy(text, SS_text);
    printf("Here we are sending mail with subject %s and body %s\n", SS_sub, SS_text);
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    curl = curl_easy_init();
    if (curl)
    {
        printf("logging in used password etc\n");
        curl_easy_setopt(curl, CURLOPT_URL, url_name);
        recipients = curl_slist_append(recipients, email_name);
        if (strlen(user) == 0)
        {
            printf("No user\n");
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        }
        if (strlen(pswd) == 0)
        {
            printf("No password\n");
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_PASSWORD, pswd);
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        }
        int flag = 0;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, flag);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);
        flag = res != CURLE_OK;
        if (res != CURLE_OK)
        {
            printf("The curl failed due to to an error\n");
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {

            curl_slist_free_all(recipients);
            curl_easy_cleanup(curl);
        }
    }
}

lld check_file(chr a[])
{
    lld cntpnt = 0;
    chr bb[300];
    memset(bb, 0, sizeof(bb));
    for (lld i = 0; i < 300; ++i)
    {
        bb[i] = 0;
    }
    int strlena = strlen(a);
    for (lld i = 0; i < strlena; ++i)
    {
        if (a[i] == '/')
        {
            ++cntpnt;
            if (cntpnt == 2)
            {
                strcpy(bb, a + i);
                break;
            }
        }
    }
    if (cntpnt != 2)
    {
        strcpy(bb, a);
    }
    for (lld i = 0; i <= files_c; i++)
    {
        chr *res_a = calloc(strlen(all_files[i]) + 2, sizeof(chr));
        strcat(res_a, "/");
        strcat(res_a, all_files[i]);
        if (strcmp(res_a, bb) == 0)
        {
            return i;
        }
        free(res_a);
    }
    return -1;
}

lld check_folder(chr a[])
{

    for (lld i = 0; i <= fldr_i; ++i)
    {
        chr *res_a = calloc(strlen(Folder[i]) + 1 + strlen("/"), sizeof(chr));
        res_a[0] = "/";
        strcat(res_a, Folder[i]);

        // printf("%d %s %s %d %s %d\n", strlen(all_files[i]), all_files[i], res_a, strlen(res_a), a, strlen(a));
        bool flag = strcmp(res_a, a) == 0;
        if (flag)
        {
            printf("\n\n folder %s \n\n", a);
            return i;
        }
        free(res_a);
    }
    return -1;
}

static lld SS_getattr(const chr *path_x, struct stat *st)
{
    st->st_uid = getuid();
    st->st_gid = getgid();
    chr path[300];
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }
    st->st_atime = st->st_mtime = time(NULL);

    // bruh

    strcpy(path, path_x);
    if (strlen(path_x))
    {
        printf("The path is %s", path_x);
    }

    if (strcmp(path, "/") == 0)
    {
        st->st_nlink = 2;
        st->st_mode = S_IFDIR | 0755;
        return 0;
    }
    else if (check_folder(path) != -1)
    {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    }
    else if (check_file(path) != -1)
    {
        st->st_mode = S_IFREG | 0644;
        st->st_size = 1024;
        st->st_nlink = 1;
        return 0;
    }
    else
    {
        return -ENOENT;
    }
    return 0;
}

static lld SS_readdir(const chr *path_x, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    printf("The list of files on path = %s\n", path_x);

    chr path[300];
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }

    strcpy(path, path_x);

    filler(buffer, ".", NULL, 0);  // Current Directory
    filler(buffer, "..", NULL, 0); // Parent Directory

    if (path[0] == '/') // If the user is trying to show the files/directories of the root directory show the following
    {
        for (lld i = 0; i <= fldr_i; ++i)
        {
            filler(buffer, Folder[i], NULL, 0);
        }
        for (lld i = 0; i <= files_r_i; ++i)
        {
            filler(buffer, Files_in_r[i], NULL, 0);
        }
    }
    else
    {
        lld dir_loc = check_folder(path);
        for (lld i = 0; i < curr_ind[dir_loc]; ++i)
        {
            filler(buffer, Files_in_d[dir_loc][i], NULL, 0);
        }
    }

    return 0;
}

static lld SS_read(const chr *path_x, chr *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
    chr path[300];
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }
    strcpy(path, path_x);
    lld cntpnt = 0;
    chr read_sub[300];
    lld s_dash_pos = -1;

    for (lld i = 0; i < strlen(path); ++i)
    {
        if (path[i] == '/')
        {
            ++cntpnt;
            if (cntpnt == 2)
            {
                s_dash_pos = i;
                break;
            }
        }
    }
    if (cntpnt == 1)
    {
        for (lld i = 0; i <= files_r_i; ++i)
        {
            int req_size = strlen(Files_in_r[i]) + strlen("/") + 1;
            chr *res_a = calloc(strlen(Files_in_r[i]) + strlen("/") + 1, sizeof(chr));
            int strlen_a = strlen(res_a);
            for (int i = 0; i < strlen_a; ++i)
            {
                res_a[i] = 0;
            }
            strcat(res_a, "/");
            strcat(res_a, Files_in_r[i]);
            bool flag = strcmp(res_a, path) == 0;
            if (flag)
            {
                strcpy(read_sub, sub_in_r[i]);
                printf("%s", read_sub);
                break;
            }
            free(res_a);
        }
    }
    else
    {
        chr _fldr[300];
        chr _file[300];
        for (lld i = 0; i < 300; ++i)
        {
            _file[i] = 0;
        }
        for (lld i = 0; i < 300; ++i)
        {
            _fldr[i] = 0;
        }
        for (lld i = 0; i < s_dash_pos; ++i)
        {
            _fldr[i] = path[i];
        }
        for (lld i = s_dash_pos + 1; i < strlen(path); ++i)
        {
            _file[i - s_dash_pos - 1] = path[i];
        }
        lld dir_loc = check_folder(_fldr);

        lld file_loc = file_in_dir(_file, dir_loc);
        strcpy(read_sub, sub_in_d[dir_loc][file_loc]);
        printf("\n%s %s %d\n", _file, _fldr, s_dash_pos);
    }
    if (strlen(read_sub))
    {
        printf("The subject is %s\n", read_sub);
    }
    get_body(read_sub);
    memcpy(buffer, _BODY, 4096);

    return strlen(_BODY);
}

static lld SS_write(const chr *path_x, const chr *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
    chr path[300];
    memset(path, 0, 300);
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }
    strcpy(path, path_x);
    lld cntpnt = 0;
    chr read_sub[300];
    lld s_dash_pos = -1;
    lld strlenpath = strlen(path);
    for (lld i = 0; i < strlenpath; i++)
    {
        if (path[i] == '/')
        {
            cntpnt++;
            if (cntpnt == 2)
            {
                s_dash_pos = i;
                break;
            }
        }
    }
    if (cntpnt == 1)
    {
        for (lld i = 0; i <= files_r_i; i++)
        {
            chr *res_a = calloc(strlen(Files_in_r[i]) + 2, sizeof(chr));
            strcat(res_a, "/");
            strcat(res_a, Files_in_r[i]);
            bool flag = strcmp(res_a, path) == 0;
            if (flag)
            {
                strcpy(read_sub, sub_in_r[i]);
                printf("%s", read_sub);
                break;
            }
            free(res_a);
        }
    }
    else
    {
        chr _fldr[300], _file[300];
        memset(_file, 0, 300);
        for (lld i = 0; i < 300; ++i)
        {
            _file[i] = 0;
        }
        memset(_fldr, 0, 300);
        for (lld i = 0; i < 300; ++i)
        {
            _fldr[i] = 0;
        }
        for (lld i = 0; i < s_dash_pos; i++)
        {
            _fldr[i] = path[i];
        }
        for (lld i = s_dash_pos + 1; i < strlen(path); i++)
        {
            _file[i - s_dash_pos - 1] = path[i];
        }
        lld dir_loc = check_folder(_fldr);

        lld file_loc = file_in_dir(_file, dir_loc);
        strcpy(read_sub, sub_in_d[dir_loc][file_loc]);
        if (file_loc == -1)
        {
            printf("File not found");
        }
    }
    get_body(read_sub);
    delete_sub(read_sub);
    if (size > 1000)
    {
        printf("\n\nWARNING: file size exceeded!!\n\n");
        exit(-1);
    }
    chr BODY_COPY[1024];
    memset(BODY_COPY, 0, 1024);
    for (lld i = 0; i < 1024; ++i)
    {
        BODY_COPY[i] = 0;
    }
    chr *x = trim(_BODY);
    strcpy(BODY_COPY, x);
    strcat(BODY_COPY, buffer);
    send_mail(read_sub, BODY_COPY);
    return size;
}

static lld SS_truncate(const chr *path_x, off_t size)
{
    int path_xlen=strlen(path_x);
    chr path[path_xlen];
    for (lld i = 0; i < path_xlen; ++i)
    {
        path[i] = 0;
    }
    strcpy(path, path_x);
    lld cntpnt = 0;
    chr read_sub[300];
    lld s_dash_pos = -1;
    lld strlenpath_ = strlen(path);
    for (lld i = 0; i < strlenpath_; i++)
    {
        if (path[i] == '/')
        {
            cntpnt += 1;
            if (cntpnt == 2)
            {
                s_dash_pos = i;
                break;
            }
        }
    }
    if (cntpnt == 1)
    {
        for (lld i = 0; i <= files_r_i; i++)
        {
            chr *res_a = calloc(strlen(Files_in_r[i]) + 2, sizeof(chr));
            strcat(res_a, "/");
            strcat(res_a, Files_in_r[i]);
            bool check = strcmp(res_a, path) == 0;
            if (check)
            {
                strcpy(read_sub, sub_in_r[i]);
                printf("%s", read_sub);
                break;
            }
            free(res_a);
        }
    }
    else
    {
        chr _fldr[300], _file[300];
        memset(_file, 0, 300);
        for (lld i = 0; i < 300; ++i)
        {
            _file[i] = 0;
        }
        memset(_fldr, 0, 300);
        for (lld i = 0; i < 300; ++i)
        {
            _fldr[i] = 0;
        }
        for (lld i = 0; i < s_dash_pos; i++)
        {
            _fldr[i] = path[i];
        }
        for (lld i = s_dash_pos + 1; i < strlen(path); i++)
        {
            _file[i - s_dash_pos - 1] = path[i];
        }
        lld dir_loc = check_folder(_fldr);

        lld file_loc = file_in_dir(_file, dir_loc);
        strcpy(read_sub, sub_in_d[dir_loc][file_loc]);
        if (file_loc != -1)
        {

            printf("file is %s", _file);
        }
        else
        {
            printf("file not found");
        }
    }
    get_body(read_sub);
    delete_sub(read_sub);
    if (size > 1024)
    {
        printf("\n\nWARNING: file size exceeded!!\n\n");
        exit(-1);
    }
    chr BODY_COPY[1024];
    for (lld i = 0; i < 1024; i++)
    {
        BODY_COPY[i] = '\0';
    }

    chr *x = trim(_BODY);
    printf("This is what we read %s\n", _BODY);
    bool xlen = strlen(x);
    if (size < xlen)
    {

        *(x + size) = 0;
    }
    strcpy(BODY_COPY, x);
    send_mail(read_sub, BODY_COPY);

    return 0;
}
int debug(int x)
{
    printf("The debugged integer is %d", x);
}
static struct fuse_operations operations = {
    .read = SS_read,
    .getattr = SS_getattr,
    .write = SS_write,
    .truncate = SS_truncate,
    .readdir = SS_readdir,
};

lld main(lld argc, chr **argv)
{
    if (argc != 3)
    {
        printf("./a.out -f [File mount path]\n");
        exit(-1);
    }
    // argv[1] mount file
    // argv[2] file
    FILE *f;
    lld buflen = 128;
    chr buf[buflen];

    f = fopen("configure.txt", "r");
    memset(buf, 0, sizeof(buf));
    for (lld i = 0; i < buflen; ++i)
    {
        buf[i] = 0;
    }
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
    for (lld i = 0; i < buflen; ++i)
    {
        buf[i] = 0;
    }
    fgets(buf, buflen, f);
    // smtp
    strcpy(smtp, buf);
    if (strcmp(smtp, "") == 0)
    {
        printf("Please enter your smtp server\n");
        exit(-1);
    }
    else{
        int length=strlen(smtp);
        url_name=(char*)malloc(length+1);
        strcpy(url_name,smtp);
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
    else{
        email_name=(char *)malloc(strlen(user)+1);
        strcpy(email_name,user);
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
    initialize();
    fclose(f);

    Smail();

    printf("\n------------------------------\n");

    return fuse_main(argc, argv, &operations, NULL);
}
/*
gcc fs.c `pkg-config fuse --cflags --libs` -lcurl
./a.out  -f store

ls
cat bro.txt
echo "hello" > bro.txt
cd folder
cd ..
ll
l
*/
