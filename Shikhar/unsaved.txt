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
    bool _isEmpty = isEmpty;
    if (!isEmpty)
    {

        return r_t(l_t(a));
    }
    else
    {
        *a = 0;
        return a;
    }
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    chr *ptr = realloc(mem->memory, mem->size + realsize + 1);
    bool _ptr = !ptr;
    if (_ptr)
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

lld searchbysubject(chr a[])
{
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chr *res_s = calloc(strlen(a) + 21 + 2, sizeof(chr));
    copystring(res_s, "UID SEARCH SUBJECT \"");
    copystring(res_s, a);
    copystring(res_s, "\"");

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
    lld val = -1;
    bool _res = res != CURLE_OK;
    if (_res)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        chr *p = chunk.memory;
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

    memset(number, 0, 5);
    sprintf(number, "%d", UID);

    chr *res_s = calloc(strlen(number) + 39, sizeof(chr));
    copystring(res_s, "imaps://imap.gmail.com:993/INBOX/;UID=");
    copystring(res_s, number);

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
    lld val;
    bool flag = res != CURLE_OK;
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
        // copystring(match_s2, "IMP");
        chr *st = strstr(chunk.memory, match_s);
        lld ind;
        if (st != NULL)
        {
            ind = st - p;
            ind += strlen(match_s);
        }
        else
        {
            st = strstr(chunk.memory, match_s2);
            ind = st - p;
            ind += strlen(match_s2);
        }
        chr line[1024];
        for (lld i = 0; i < 1024; ++i)
        {
            line[i] = 0;
        }
        memset(line, 0, 1024);
        lld k = 0;

        for (lld i = ind; chunk.memory[i] != '-' && i < strlen(chunk.memory); i++)
        {
            if (chunk.memory[i] == '/' || chunk.memory[i] == '.' || chunk.memory[i] == '=' || isalnum(chunk.memory[i]))
            {
                line[k++] = chunk.memory[i];
            }
            else
            {
                k = 0;
                if (strlen(line) > 0)
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

                        for (lld i = 0; i < strlen(line); i++)
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

                        for (lld i = 0; i < strlen(x1); i++)
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
                            lld dir_loc = find_dir(y1);
                            if (dir_loc == -1)
                            {
                                printf("corrupted file\n");
                                exit(-1);
                            }
                            strcpy(Files_in_d[dir_loc][curr_ind[dir_loc]], y2);
                            strcpy(sub_in_d[dir_loc][curr_ind[dir_loc]], x2);
                            files_c++;
                            strcpy(all_files[files_c], y2);
                            curr_ind[dir_loc]++;
                            printf("Files dir %s %s %s\n", y1, y2, x2);
                        }
                        else
                        {
                            files_r_i++;
                            x1++;
                            strcpy(Files_in_r[files_r_i], x1);
                            strcpy(sub_in_r[files_r_i], x2);
                            files_c++;
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
    copystring(res_s, "imaps://imap.gmail.com:993/INBOX/;UID=");
    copystring(res_s, number);

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
    lld val;
    chr *mail_body_text = calloc(1024, sizeof(chr));
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else
    {
        chr *p = chunk.memory;
        chr *match_s = calloc(11 + strlen(read_sub), sizeof(chr));
        copystring(match_s, "Subject: ");
        copystring(match_s, read_sub);
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

        for (lld i = ind; i < strlen(chunk.memory); i++)
        {
            if (chunk.memory[i] != '\n')
            {
                line[k++] = chunk.memory[i];
            }
            else
            {
                k = 0;
                if (strlen(line) > 0)
                {
                    copystring(mail_body_text, line);
                    copystring(mail_body_text, "\n");
                    memset(line, 0, 1024);
                    for (lld i = 0; i < 1024; ++i)
                    {
                        line[i] = 0;
                    }
                }
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

    chr *res_s = calloc(strlen(number) + 11 + 19, sizeof(chr));
    copystring(res_s, "UID STORE ");
    copystring(res_s, number);
    copystring(res_s, " +FLAGS (\\Deleted)");

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
    lld val;
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
    memset(data, 0, 1024);
    for (lld i = 0; i < 1024; ++i)
    {
        data[i] = 0;
    }

    if (size * nmemb < 1)
        return 0;

    if (upload_ctx->lines_read == 0)
    {
        // subject
        strcpy(data, sub_);
    }
    else if (upload_ctx->lines_read == 1 || upload_ctx->lines_read == 3)
    {
        strcpy(data, "\r\n");
    }
    else if (upload_ctx->lines_read == 4)
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
        upload_ctx->lines_read++;
        return len;
    }
    return 0;
}

void send_mail(chr SS_sub[], chr SS_text[])
{
    chr *res_s = calloc(strlen(SS_sub) + 10, sizeof(chr));
    copystring(res_s, "Subject: ");
    copystring(res_s, SS_sub);
    strcpy(sub_, res_s);
    strcpy(text, SS_text);

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
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
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

lld check_file(chr a[])
{
    lld cntpnt = 0;
    chr bb[300];
    memset(bb, 0, sizeof(bb));
    for (lld i = 0; i < 300; ++i)
    {
        bb[i] = 0;
    }

    for (lld i = 0; i < strlen(a); i++)
    {
        if (a[i] == '/')
        {
            cntpnt += 1;
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
        copystring(res_a, "/");
        copystring(res_a, all_files[i]);
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

    for (lld i = 0; i <= fldr_i; i++)
    {
        chr *res_a = calloc(strlen(Folder[i]) + 2, sizeof(chr));
        copystring(res_a, "/");
        copystring(res_a, Folder[i]);

        // printf("%d %s %s %d %s %d\n", strlen(all_files[i]), all_files[i], res_a, strlen(res_a), a, strlen(a));
        if (strcmp(res_a, a) == 0)
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
    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);

    // bruh
    chr path[300];
    memset(path, 0, 300);
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }

    strcpy(path, path_x);

    printf("path asked for %s\n", path_x);

    if (strcmp(path, "/") == 0)
    {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    }
    else if (check_folder(path) != -1)
    {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    }
    else if (check_file(path) != -1)
    {
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = 1024;
    }
    else
    {
        return -ENOENT;
    }
    return 0;
}

static lld SS_readdir(const chr *path_x, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    printf("--> Getting The List of Files of %s\n", path_x);

    chr path[300];
    memset(path, 0, 300);
    for (lld i = 0; i < 300; ++i)
    {
        path[i] = 0;
    }

    strcpy(path, path_x);

    filler(buffer, ".", NULL, 0);  // Current Directory
    filler(buffer, "..", NULL, 0); // Parent Directory

    // return 0;
    if (strcmp(path, "/") == 0) // If the user is trying to show the files/directories of the root directory show the following
    {
        for (lld i = 0; i <= fldr_i; i++)
        {
            filler(buffer, Folder[i], NULL, 0);
        }
        for (lld i = 0; i <= files_r_i; i++)
        {
            filler(buffer, Files_in_r[i], NULL, 0);
        }
    }
    else
    {
        lld dir_loc = check_folder(path);
        for (lld i = 0; i < curr_ind[dir_loc]; i++)
        {
            filler(buffer, Files_in_d[dir_loc][i], NULL, 0);
        }
    }

    return 0;
}

static lld SS_read(const chr *path_x, chr *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
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

    for (lld i = 0; i < strlen(path); i++)
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
            copystring(res_a, "/");
            copystring(res_a, Files_in_r[i]);
            if (strcmp(res_a, path) == 0)
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
        printf("\n%s %s %d\n", _file, _fldr, s_dash_pos);
    }
    printf("\nThis is the sub = %s\n", read_sub);
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

    for (lld i = 0; i < strlen(path); i++)
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
            copystring(res_a, "/");
            copystring(res_a, Files_in_r[i]);
            if (strcmp(res_a, path) == 0)
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
    copystring(BODY_COPY, buffer);
    send_mail(read_sub, BODY_COPY);
    return size;
}

static lld SS_truncate(const chr *path_x, off_t size)
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

    for (lld i = 0; i < strlen(path); i++)
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
            copystring(res_a, "/");
            copystring(res_a, Files_in_r[i]);
            if (strcmp(res_a, path) == 0)
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
        printf("\n%s %s %d\n", _file, _fldr, s_dash_pos);
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
    memset(BODY_COPY, 0, 1024);
    for (lld i = 0; i < 1024; i++)
    {
        BODY_COPY[i] = '\0';
    }

    chr *x = trim(_BODY);
    printf("This is what we read %s\n", _BODY);
    if (size < strlen(x))
    {

        *(x + size) = 0;
    }
    strcpy(BODY_COPY, x);
    send_mail(read_sub, BODY_COPY);

    return 0;
}

static struct fuse_operations operations = {
    .getattr = SS_getattr,
    .readdir = SS_readdir,
    .read = SS_read,
    .write = SS_write,
    .truncate = SS_truncate,
    // .mkdir		= do_mkdir,
    // .mknod		= do_mknod,
    // .rm
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


cat bro.txt
echo "hello" > bro.txt
*/