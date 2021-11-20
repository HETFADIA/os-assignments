/**
 * Less Simple, Yet Stupid Filesystem.
 * 
 * Mohammed Q. Hussain - http://www.maastaar.net
 *
 * This is an example of using FUSE to build a simple filesystem. It is a part of a tutorial in MQH Blog with the title "Writing Less Simple, Yet Stupid Filesystem Using FUSE in C": http://maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/
 *
 * License: GNU GPL
 */
 
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

struct string_ptr {
  char *ptr;
  size_t len;
};

#define userEmail "chourasiaaman9000@gmail.com"
#define userPassword "anamndainni"

void init_string_ptr(struct string_ptr *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string_ptr *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

char dir_list[ 256 ][ 256 ];
int curr_dir_idx = -1;

char files_list[ 256 ][ 256 ];
int curr_file_idx = -1;

char files_content[ 256 ][ 256 ];
int curr_file_content_idx = -1;

void add_dir( const char *dir_name )
{
	curr_dir_idx++;
	strcpy( dir_list[ curr_dir_idx ], dir_name );
}

int is_dir( const char *path )
{

	int len=(int)strlen(path)+6;
	char pattern[len];

	pattern[0] = '"'; 
	pattern[1] = '/';
	pattern[2] = '"';
	pattern[3] = ' ';
	pattern[4] = '"';

	char *t;
	int first_slash = 1;
	int index = 5;
	for(t = path; *t != '\0'; t++){
		if(*t == '/' && first_slash == 1){
			first_slash = 0;
			continue;
		}
		pattern[index] = *t; index++;
	}

	pattern[index] = '"';
	index++;
	pattern[index] = '\0';

	CURL *curl;
    curl = curl_easy_init();
	CURLcode res = CURLE_OK;
	struct string_ptr s;
    if(curl) {
    	init_string_ptr(&s);
		/* Set username and password */
		curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
	
		/* This will list the folders within the user's mailbox. If you want to
		* list the folders within a specific folder, for example the inbox, then
		* specify the folder as a path in the URL such as /INBOX */
		curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
	
		/* Perform the list */
		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		
		/* Always cleanup */
		curl_easy_cleanup(curl);
    }

	for(int i = 0; i < (int)strlen(s.ptr); i++){
		int curr = i;
		int matched_len = 0;
		for(t = pattern; *t != '\0'; t++){
			if(s.ptr[curr] != *t){
				break;
			}
			curr++;
			matched_len++;
			if(matched_len == len - 1) return 1;
		}
	}
    return 0;
}

void add_file( const char *filename )
{
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}

int is_file( const char *path )
{
	path++; // Eliminating "/" in the path
	
	char directoryName[100];
	directoryName[0] = '/';
	int index = 1;
	char *t;
	int firstSlash = 1;
	for(t = path; *t != '\0'; t++){
		if(*t == '/' && firstSlash == 1){
			if(directoryName[0] != '['){
				break;
			}
			directoryName[index++] = *t;
			firstSlash = 0;
			continue;
		}
		else if(*t == '/') break;
		directoryName[index] = *t;
		index += 1;
	}

	directoryName[index] = '\0';
	if(is_dir(directoryName) == 0) return 0;


	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int get_file_index( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}

void write_to_file( const char *path, const char *new_content )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
		
	strcpy( files_content[ file_idx ], new_content ); 
}

static int do_getattr( const char *path, struct stat *st )
{
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	printf("\n\nIn do_getattr - %s\n\n",path);

	if ( strcmp( path, "/" ) == 0 || is_dir(path) == 1)
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else if ( is_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}

	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
    CURL *curl;
    curl = curl_easy_init();
	CURLcode res = CURLE_OK;
    if(curl) {
        struct string_ptr s;
    	init_string_ptr(&s);
		/* Set username and password */
		curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
	
		/* This will list the folders within the user's mailbox. If you want to
		* list the folders within a specific folder, for example the inbox, then
		* specify the folder as a path in the URL such as /INBOX */
		curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
	
		/* Perform the list */
		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		
		const char* usethis = malloc((int)strlen(s.ptr));
		usethis = s.ptr;
		filler( buffer, usethis, NULL, 0 );
    	free(s.ptr);

		/* Always cleanup */
		curl_easy_cleanup(curl);
    }
    
    return 0;
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 )
		return -1;
	
	char *content = files_content[ file_idx ];
	
	memcpy( buffer, content + offset, size );
		
	return strlen( content ) - offset;
}

static int do_mkdir( const char *path, mode_t mode )
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	
	char command[7 + (int)strlen(path)];
	command[0] = 'C';
	command[1] = 'R';
	command[2] = 'E';
	command[3] = 'A';
	command[4] = 'T';
	command[5] = 'E';
	command[6] = ' ';

	int index = 7;
	char *t;
	for(t = path; *t != '\0'; t++){
		if(*t == '/') continue;
		command[index] = *t;
		index += 1;
	}

	command[index] = '\0';

	curl = curl_easy_init();
	if(curl) {
		/* Set username and password */
		curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
	
		/* This is just the server URL */
		curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");
	
		/* Set the CREATE command specifying the new folder name */
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command);
	
		/* Perform the custom request */
		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	
		/* Always cleanup */
		curl_easy_cleanup(curl);
	}
 
  	return 0;
}

static int do_rmdir( const char *path )
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	
	char command[7 + (int)strlen(path)];
	command[0] = 'D';
	command[1] = 'E';
	command[2] = 'L';
	command[3] = 'E';
	command[4] = 'T';
	command[5] = 'E';
	command[6] = ' ';

	int index = 7;
	char *t;
	for(t = path; *t != '\0'; t++){
		if(*t == '/') continue;
		command[index] = *t;
		index += 1;
	}

	command[index] = '\0';

	curl = curl_easy_init();
	if(curl) {
		/* Set username and password */
		curl_easy_setopt(curl, CURLOPT_USERNAME, "2019csb1067@iitrpr.ac.in");
		curl_easy_setopt(curl, CURLOPT_PASSWORD, "anamndainni");
	
		/* This is just the server URL */
		curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com");
	
		/* Set the DELETE command specifying the existing folder */
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command);
	
		/* Perform the custom request */
		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	
		/* Always cleanup */
		curl_easy_cleanup(curl);
	}
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev )
{
	path++;
	add_file( path );
	
	return 0;
}

static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	write_to_file( path, buffer );
	
	return size;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
    .mkdir		= do_mkdir,
    .mknod		= do_mknod,
    .write		= do_write,
	.rmdir      = do_rmdir,
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &operations, NULL );
}