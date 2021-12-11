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

    bool _isalnum;
    for (lld i = 0; i < strlena; i++)
    {
        _isalnum = ('0' <= a[i] && a[i] <= '9') || ('a' <= a[i] && a[i] <= 'z') || ('A' <= a[i] && a[i] <= 'Z');
        if (_isalnum)
        {
            isEmpty = 0;
        }
    }
    bool _isEmpty = !isEmpty;
    if (_isEmpty)
    {

        return r_t(l_t(a));
    }
    a[0] = '\0';
    return a;
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    size_t realsize = size * nmemb;
    int ptrsize = mem->size + realsize + 1;
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
    int lastindex = mem->size;
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
    char *uid_search_sub = "UID SEARCH SUBJECT \"";
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
    printf("%d", _res);
    if (_res)
    {
        printf("The curl did not get executed due to some error\n");
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
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
chr Folder[1500][300];
lld curr_ind[1500];
chr Files_in_d[1500][1500][300];
chr Files_in_r[1500][300];
chr sub_in_r[1500][300];
chr sub_in_d[1500][1500][300];
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
void exitcode(char *s)
{
    printf("%s\n", s);
    exit(-1);
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
        chr match_s2[] = "Subject: IMP";

        lld ind;

        ind = strstr(chunk.memory, match_s2) - p + strlen(match_s2);

        chr line[1500];
        for (lld i = 0; i < 1500; ++i)
        {
            line[i] = 0;
        }
        memset(line, 0, 1500);
        lld k = 0;

        for (lld i = ind; chunk.memory[i] != '-' && i < strlen(chunk.memory); ++i)
        {
            bool a1 = chunk.memory[i] == '/';
            bool a2 = chunk.memory[i] == '.';
            bool a3 = chunk.memory[i] == '=';
            bool a4 = isalnum(chunk.memory[i]);
            bool a5 = chunk.memory[i] == '_';
            if (a1 || a2 || a3 || a4 || a5)
            {
                line[k++] = chunk.memory[i];
            }
            else
            {
                k = 0;
                if (strlen(line))
                {
                    printf("%s\n", line);
                    char *isFile;
                    lld strlenline = strlen(line);

                    isFile = strstr(line, "=");

                    if (isFile)
                    {
                        char *fft = line;
                        int indexa = isFile - fft;
                        lld curr = -1;
                        int strlenline = strlen(line);

                        curr = indexa;
                        chr *x1, *x2;
                        x1 = line;
                        x2 = line + curr + 1;
                        line[curr] = '\0';
                        lld cntpnt = -1;
                        lld strlenx1 = strlen(x1);
                        for (lld i = 0; i < strlenx1; ++i)
                        {
                            if (x1[i] == '/')
                            {
                                for (int j = i + 1; j < strlenx1; ++j)
                                {
                                    if (x1[j] == '/')
                                    {
                                        cntpnt = j;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        if (cntpnt != -1)
                        {
                            ++x1;
                            chr *token1 = strtok(x1, "/");
                            chr *theta1 = token1;
                            token1 = strtok(NULL, "/");
                            chr *theta2 = token1;
                            lld dir_loc = -1;

                            for (lld i = 0; i <= fldr_i; ++i)
                            {
                                if (strcmp(Folder[i], theta1) == 0)
                                {
                                    dir_loc = i;
                                    break;
                                }
                            }
                            if (dir_loc == -1)
                            {
                                exitcode("We could not find the directory\n");
                            }
                            else
                            {
                                strcpy(Files_in_d[dir_loc][curr_ind[dir_loc]], theta2);
                                printf("The data %s is copied in the file inside directory\n", theta2);
                                strcpy(sub_in_d[dir_loc][curr_ind[dir_loc]++], x2);
                                strcpy(all_files[++files_c], theta2);

                                if (strlen(theta1))
                                {
                                    printf("The folder is %s", theta1);
                                }
                                if (strlen(theta2))
                                {
                                    printf("The file is %s", theta2);
                                }
                            }
                        }
                        else
                        {
                            files_r_i = files_r_i + 1;
                            x1 = x1 + 1;
                            lld j = 0;
                            while (x1[j] != '\0')
                            {
                                Files_in_r[files_r_i][j] = x1[j];
                                ++j;
                            }

                            strcpy(sub_in_r[files_r_i], x2);

                            strcpy(all_files[++files_c], x1);
                            if (strlen(x1))
                            {
                                printf("The file is %s\n", x1);
                            }
                            if (strlen(x2))
                            {
                                printf("The data is %s\n", x2);
                            }
                        }
                    }
                    else
                    {
                        chr *char_pointer = line + 1;

                        fldr_i = fldr_i + 1;
                        lld j = 0;
                        while (char_pointer[j] != '\0')
                        {
                            Folder[fldr_i][j] = char_pointer[j];
                            ++j;
                        }
                        printf("Folder %s\n", char_pointer);
                        if (strlen(char_pointer))
                        {
                            printf("The folder we get is  %s\n", char_pointer);
                        }
                    }
                    for (lld i = 0; i < 1500; ++i)
                    {
                        line[i] = '\0';
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
    int length = 1500;
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
        chr line[1500];
        memset(line, 0, 1500);
        for (lld i = 0; i < 1500; ++i)
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

                    for (lld i = 0; i < 1500; ++i)
                    {
                        line[i] = 0;
                    }
                }
                k = 0;
            }
        }
        printf("The body is %s ", mail_body_text);
        chr *trimmed_text = trim(mail_body_text);
        memset(_BODY, 0, 1500);
        for (lld i = 0; i < 1500; ++i)
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
    char *uid_st = "UID STORE ";
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

chr text[1500];
chr sub_[30];

struct upload_status
{
    lld lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    chr data[1500];
    for (lld i = 0; i < 1500; ++i)
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
char *email_name = NULL;
char *url_name = NULL;
void send_mail(chr file_sub[], chr file_text[])
{
    CURL *curl;
    CURLcode res;
    res = CURLE_OK;
    chr *res_s = calloc(strlen(file_sub) + 10, sizeof(chr));
    chr *start = "Subject: ";
    strcat(res_s, start);
    strcat(res_s, file_sub);
    strcpy(sub_, res_s);
    strcpy(text, file_text);
    printf("Here we are sending mail with subject %s and body %s\n", file_sub, file_text);
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    curl = curl_easy_init();
    if (curl)
    {
        printf("logging in used password etc\n");
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com");
        recipients = curl_slist_append(recipients, "testhailaude@gmail.com");
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
    for (lld i = 0; i < 300; ++i)
    {
        bb[i] = 0;
    }
    chr *charpointer = a + 1;
    chr *token1 = strtok(charpointer, "/");
    chr *theta1 = token1;
    token1 = strtok(NULL, "/");
    chr *theta2 = token1;
    char *another = theta2;
    if (theta2 == NULL)
    {
        another = theta1;
    }
    lld j = 0;
    while (j <= files_c)
    {
        if (strcmp(another, all_files[j]) == 0)
        {
            return j;
        }
        j++;
    }
    return -1;
}

lld check_folder(chr a[])
{
    lld i = 0;
    char *chrptr = a;
    ++chrptr;
    while (i <= fldr_i)
    {
        if (strcmp(Folder[i], chrptr) == 0)
        {
            return i;
        }
        ++i;
    }
    return -1;
}

static lld file_getattr(const chr *path_x, struct stat *st)
{
    st->st_uid = getuid();
    st->st_gid = getgid();
    chr path[300];
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }
    st->st_atime = st->st_mtime = time(NULL);
    lld j = 0;
    while (path_x[j] != '\0')
    {
        path[j] = path_x[j];
        j++;
    }

    if (strcmp(path, "/") == 0 || check_folder(path) != -1)
    {
        st->st_nlink = 2;
        st->st_mode = S_IFDIR | 0755;
        return 0;
    }
    if (check_file(path) != -1)
    {
        st->st_mode = S_IFREG | 0644;
        st->st_size = 1500;
        st->st_nlink = 1;
        return 0;
    }

    return -ENOENT;
}

static lld file_readdir(const chr *path_x, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);
    chr path[300];
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }

    lld j = 0;
    while (path_x[j] != '\0')
    {
        path[j] = path_x[j];
        ++j;
    }

    bool isRoot = path[0] == '/';
    if (isRoot)
    {

        lld j = 0;
        while (j <= fldr_i)
        {
            filler(buffer, Folder[j], NULL, 0);
            ++j;
        }

        j = 0;
        while (j <= files_r_i)
        {
            filler(buffer, Files_in_r[j], NULL, 0);
            ++j;
        }
    }
    else
    {
        lld dir_loc = check_folder(path);

        lld j = 0;
        while (j < curr_ind[dir_loc])
        {
            filler(buffer, Files_in_d[dir_loc][j], NULL, 0);
            ++j;
        }
    }

    return 0;
}

static lld file_write(const chr *path_x, const chr *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
    chr path[300];
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }
    lld j = 0;
    while (path[j] != '\0')
    {
        path[j] = path_x[j];
        ++j;
    }
    lld cntpnt = 0;
    chr read_sub[300];
    chr *charpointer = a + 1;
    chr *token1 = strtok(charpointer, "/");
    chr *theta1 = token1;
    token1 = strtok(NULL, "/");
    chr *theta2 = token1;
    if (theta2 == NULL)
    {
        lld j=0;
        while(j<=files_r_i)
        {
            if(strcmp(theta1,Files_in_r[j])==0)
            {
                break;
            }
            j++;
        }
        if(j>files_r_i)
        {
            return -1;
        }
        lld k=0;
        while(sub_in_r[j][k]!='\0')
        {
            read_sub[k]=sub_in_r[j][k];
            k++;
        }
    }
    else{

    }
    
    get_body(read_sub);
    delete_sub(read_sub);
    if (size > 1000)
    {
        printf("\n\nWARNING: file size exceeded!!\n\n");
        exit(-1);
    }
    chr BODY_COPY[1500];
    memset(BODY_COPY, 0, 1500);
    for (lld i = 0; i < 1500; ++i)
    {
        BODY_COPY[i] = 0;
    }
    chr *x = trim(_BODY);
    strcpy(BODY_COPY, x);
    strcat(BODY_COPY, buffer);
    send_mail(read_sub, BODY_COPY);
    return size;
}

static lld file_truncate(const chr *path_x, off_t size)
{
    

    return 0;
}
int debug(int x)
{
    printf("The debugged integer is %d", x);
}
static struct fuse_operations operations = {

    .getattr = file_getattr,
    .write = file_write,
    .truncate = file_truncate,
    .readdir = file_readdir,
};

lld main(lld argc, chr **argv)
{
    if (argc != 3)
    {
        printf("./a.out -f [File mount path]\n");
        exit(-1);
    }
    // argv[1] mount file

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
    else
    {
        int length = strlen(smtp);
        url_name = (char *)malloc(length + 1);
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
        email_name = (char *)malloc(strlen(user) + 1);
        strcpy(email_name, user);
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
gcc main.c `pkg-config fuse --cflags --libs` -lcurl
./a.out  -f store

ls
cat bro.txt
echo "h" > rand.txt
cd folder
cd ..
ll
l
*/
