#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <curl/curl.h>
#define debug(n) printf("Line : %d\n",n);

char payload_text[50000];

char *label_name = "imap_test_3";
int current_uid = 1;
int master_uid = 1;

#define FROM "<pradeep.kumar.300802@gmail.com>"
#define TO "<pradeep.kumar.300802@gmail.com>"

char *convert_int_to_string(int num)
{
  int i = 0;
  char *buffer = (char *)malloc(sizeof(char) * 20);
  while (num != 0)
  {
    buffer[i++] = (char)('0' + num % 10);
    num /= 10;
  }
  buffer[i] = '\0';
  i--;
  int j = 0;
  while (j < i)
  {
    char tp = buffer[i];
    buffer[i] = buffer[j];
    buffer[j] = tp;
    i--;
    j++;
  }
  return buffer;
}

int delete_mail(char *labelName, int uid){
  printf("deleting mail with UID: %d\n", uid);
  CURL *curl;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "pradeep.kumar.300802@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "Test@1234");

    char url[100] = "imaps://imap.gmail.com/";
    strcat(url, labelName);
    strcat(url, "/");
    // printf("debugging, delete_mail url : %s\n", url);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    char tp_buffer[100] = "STORE ";
    strcat(tp_buffer, convert_int_to_string(uid));
    strcat(tp_buffer, " +FLAGS (\\DELETED)");
    // printf("debugging, delete_mail tp_buffer : %s\n", tp_buffer);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, tp_buffer);

    // curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "STORE 2 +FLAGS (\\DELETED)");

    /* Perform the custom request */
    res = curl_easy_perform(curl);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "EXPUNGE");

    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* Always cleanup */
    curl_easy_cleanup(curl);
  }
  return (int)res;
}

char *generate_message_id(size_t size)
{
  char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
  char *str = NULL;
  if (size)
  {
    str = (char *)malloc(sizeof(char) * (size + 1));
    for (size_t n = 0; n < size; n++)
    {
      int key = rand() % 64;
      str[n] = charset[key];
    }
    str[size] = '\0';
  }
  return str;
}

struct string
{
  char *ptr;
  size_t len;
};

void init_string(struct string *s)
{
  s->len = 0;
  s->ptr = (char *)malloc(sizeof(char) * (s->len + 1));
  if (s->ptr == NULL)
  {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

// size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
// {
//   size_t new_len = s->len + size * nmemb;
//   s->ptr = realloc(s->ptr, new_len + 1);
//   if (s->ptr == NULL)
//   {
//     fprintf(stderr, "realloc() failed\n");
//     exit(EXIT_FAILURE);
//   }
//   memcpy(s->ptr + s->len, ptr, size * nmemb);
//   s->ptr[new_len] = '\0';
//   s->len = new_len;
//   return size * nmemb;
// }

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size * nmemb;
  char *temp_ptr = realloc(s->ptr, new_len + 1);
  if (temp_ptr == NULL)
  {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr = temp_ptr;
  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->len += size * nmemb;
  s->ptr[s->len] = '\0';
  return size * nmemb;
}

// struct string fetch_mail(char *labelName, int uid)
// {
//   printf("Entering Fetch function, lablelName is %s and UID is %d\n",labelName, uid);
//   CURL *curl;
//   CURLcode res = CURLE_OK;

//   curl = curl_easy_init();
//   if (curl)
//   {
//     // struct string s = (struct string *)malloc(sizeof(struct string));
//     struct string s;
//     init_string(&s);

//     /* Set username and password */
//     curl_easy_setopt(curl, CURLOPT_USERNAME, "pradeep.kumar.300802@gmail.com");
//     curl_easy_setopt(curl, CURLOPT_PASSWORD, "Test@1234");

//     char url[100] = "imaps://imap.gmail.com/";
//     strcat(url, labelName);
//     strcat(url, "/;UID=");
//     strcat(url, convert_int_to_string(uid));
//     strcat(url, "/;SECTION=TEXT");

//     curl_easy_setopt(curl, CURLOPT_URL, url);
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

//     /* Perform the fetch */
//     res = curl_easy_perform(curl);

//     /* Check for errors */
//     if (res != CURLE_OK)
//       fprintf(stderr, "curl_easy_perform() failed: %s\n",
//               curl_easy_strerror(res));

//     curl_easy_cleanup(curl);
//     printf("Inside fetch_email function printing data of struct s\n");
//     printf("s.len : %zu, s.ptr: %s\n",s.len,s.ptr);  
//     return s;
//   }
//   struct string basic;
//   basic.len = -1;
//   return basic;
// }

struct string fetch_mail(char *labelName, int uid)
{
  printf("Entering Fetch function, lablelName is %s and UID is %d\n",labelName, uid);
  CURL *curl;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    struct string* s = (struct string *)malloc(sizeof(struct string));
    // struct string s;
    init_string(s);

    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "pradeep.kumar.300802@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "Test@1234");

    char url[100] = "imaps://imap.gmail.com/";
    strcat(url, labelName);
    strcat(url, "/;UID=");
    strcat(url, convert_int_to_string(uid));
    strcat(url, "/;SECTION=TEXT");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    /* Perform the fetch */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    // printf("Inside fetch_email function printing data of struct s\n");
    // printf("s.len : %zu, s.ptr: %s\n",s->len,s->ptr);  
    return *s;
  }
  struct string basic;
  basic.len = -1;
  return basic;
}

struct upload_status
{
  size_t bytes_read;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;

  if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
  {
    return 0;
  }

  data = &payload_text[upload_ctx->bytes_read];

  if (*data)
  {
    size_t len = strlen(data);
    if (room < len)
      len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;

    return len;
  }

  return 0;
}

int send_email(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();
  if (curl)
  {
    long infilesize;
    struct upload_status upload_ctx = {0};

    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "pradeep.kumar.300802@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "Test@1234");

    /* This will create a new message 100. Note that you should perform an
     * EXAMINE command to obtain the UID of the next message to create and a
     * SELECT to ensure you are creating the message in the OUTBOX. */
    char url[100] = "imaps://imap.gmail.com/";
    strcat(url, label_name);
    // printf("send_email url: %s\n", url);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    debug(1);

    /* In this case, we are using a callback function to specify the data. You
     * could just use the CURLOPT_READDATA option to specify a FILE pointer to
     * read from. */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    debug(2);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    debug(3);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    debug(4);

    infilesize = strlen(payload_text);
    debug(5);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, infilesize);
    debug(6);

    // printf("Inside send_email ,Payload Text : %s\nPayload Text Size(infilesize): %ld\n",payload_text, infilesize);
    printf("Inside send_email ,Payload Text Size(infilesize): %ld\n", infilesize);
    /* Perform the append */
    res = curl_easy_perform(curl);
    debug(7);

    /* Check for errors */
    if (res != CURLE_OK){
      debug(8);
      fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
      debug(9);
    }
    debug(10);
    /* Always cleanup */
    curl_easy_cleanup(curl);
    debug(11);
  }

  return (int)res;
}

void create_master_email(char *body){
  printf("inside master email uid: %d\n",current_uid);
  // strcpy(payload_text, "To: " TO "\r\n"
  //                      "From: " FROM "(Example User)\r\n"
  //                      "Cc: " FROM "(Another example User)\r\n"
  //                      "Message-ID: ");
  // strcat(payload_text, "<");
  // char *stringUID = convert_int_to_string(current_uid);

  // strcat(payload_text, stringUID);

  // strcat(payload_text, ">\r\n");

  // strcat(payload_text, "Subject: ");
  // strcat(payload_text, "Master File");
  // strcat(payload_text, "\r\n");
  // strcat(payload_text, "\r\n"); /* empty line to divide headers from body, see RFC5322 */
  // strcat(payload_text, body);
  // strcat(payload_text, "\r\n");

  strcpy(payload_text,"Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n"
  "To: " TO "\r\n"
  "From: " FROM "(Example User)\r\n"
  "Cc: " FROM "(Another example User)\r\n"
  "Message-ID: "
  "<6>\r\n"
  "Subject: Master Email\r\n"
  "\r\n" /* empty line to divide headers from body, see RFC5322 */
  "The body of the message starts here.\r\n"
  "\r\n"
  "apurv.txt:0i-abQiBjSc5zvgQ0GX6VEIDPw3WfhK5qH3pm9QvRQof-sTXYF:2\r\n"
  "vasu.txt:0i-abQiBjSc5zvgQ0GX6VEIDPw3WfhK5qH3pm9QvRQof-sTXYF:5.\r\n");


  // printf("Printing payload_text: %s\n", payload_text);

  // printf("%s\n",payload_text);

  send_email();

  printf("End of create Master email file\n");
  // printf("Abhi bhi nahi chal raha!!!\n");
}

void create_new_email(const char *filename, char *body){
  printf("inside create_new_email uid: %d\n",current_uid);
  if(current_uid == 1){
		// create master file
    printf("Creating Master File for the first time\n");
		create_master_email("");                     
		current_uid++;
	}
  char *message_id = generate_message_id(50);

  strcpy(payload_text, "To: " TO "\r\n"
                       "From: " FROM "(Example User)\r\n"
                       "Cc: " FROM "(Another example User)\r\n"
                       "Message-ID: ");
  strcat(payload_text, "<");
  char *stringUID = convert_int_to_string(current_uid);

  strcat(payload_text, stringUID);

  strcat(payload_text, ">\r\n");

  strcat(payload_text, "Subject: ");
  strcat(payload_text, message_id);
  strcat(payload_text, "\r\n");
  strcat(payload_text, "\r\n"); /* empty line to divide headers from body, see RFC5322 */
  strcat(payload_text, body);
  strcat(payload_text, "\r\n");
  // printf("Printing payload_text: %s\n", payload_text);
  send_email();
  printf("after send_email in create_new_email function\n");

  char new_line[300] = "";

  printf("filename: %s\n", filename);
  strcat(new_line, filename);
  strcat(new_line, ":");
  strcat(new_line, message_id);
  strcat(new_line, ":");
  strcat(new_line, convert_int_to_string(current_uid));
  strcat(new_line, "\r\n");

  // printf("in create_new_email function, new_line: %s\n",new_line);
  // Update the master file here
  struct string master_body = fetch_mail(label_name, master_uid);
  // printf("Inside create_new_email function around line no 395 printing data of struct master body\n");
  // printf("master_body.len : %zu, master_body.ptr: %s\n",master_body.len,master_body.ptr);
  if (master_body.len == -1)
  {
    printf("Error in fetching email with uid:%d\n", master_uid);
  }
  // else{
  //   printf("Content in master file:%s\n",master_body.ptr);
  // }
  strcat(master_body.ptr, new_line);
  // printf("Content in master file after concatenation:%s\n",master_body.ptr);
  current_uid++;
  delete_mail(label_name, master_uid);
  printf("Finally Deleted Email with uid: %d\n",master_uid);
  create_master_email(master_body.ptr);
  // printf("Master Email has been sucessfully created\n");
  master_uid = current_uid++;
  printf("End of create new email file with filename: %s\n", filename);
  // printf("Abhi bhi nahi chal raha!!!\n");
}

void decode_body(char *body)
{
  int n = strlen(body);
  int first = 0;
  int cap = 2000;
  char *filename = (char *)malloc(sizeof(char) * cap);
  char *message_id = (char *)malloc(sizeof(char) * cap);
  char *uid_str = (char *)malloc(sizeof(char) * cap);
  int uid;
  for (int i = 0; i < cap; i++)
  {
    filename[i] = '\0';
    message_id[i] = '\0';
    uid_str[i] = '\0';
  }
  int j = 0;
  for (int i = 0; i < n; i++)
  {
    if (body[i] == ':')
    {
      first++;
      j = 0;
    }
    else
    {
      if (first == 0)
      {
        filename[j++] = body[i];
      }
      else if (first == 1)
      {
        message_id[j++] = body[i];
      }
      else
      {
        uid_str[j++] = body[i];
      }
    }
  }
  // convert string to int
  uid = atoi(uid_str);
  printf("inside decode_body: filename: %s\n", filename);
  printf("inside decode_body: message_id: %s\n", message_id);
  printf("inside decode_body: uid_str: %s\n", uid_str);
  printf("inside decode_body: uid: %d\n", uid);
}

int main()
{
  srand(time(0));
  printf("Enter current_uid: ");
  scanf("%d",&current_uid);
  printf("Enter master_uid: ");
  scanf("%d",&master_uid);
  // while(getchar() != '\n'); 
  // struct string ans = fetch_mail(label_name,20);
  // printf("%s\n",ans.ptr);
  char *body = "";
//   create_master_email(body);
  char *file_name = "apurv.txt";
  create_new_email(file_name, body);

  // int op;
  // while(1){
  //     scanf("%d",&op);
  //     while ((getchar()) != '\n');
  //     if(op==0){
  //         break;
  //     }
  //     else if(op==1){
  //         // create email
  //         char *file_name;
  //         file_name = (char*) malloc(1024 * sizeof(char));
  //         bzero(file_name, 1024);
  //         fgets(file_name, 1024, stdin);
  //         while ((getchar()) != '\n');
  //         create_new_email(file_name,"");
  //     }
  // }

  return 0;
}