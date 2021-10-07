// C program to demonstrate error when a
// process tries to access resources beyond
// limit.
#include <stdio.h>
#include <sys/resource.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {

	struct rlimit old_lim, lim, new_lim;

	// Get old limits
	if( getrlimit(RLIMIT_NOFILE, &old_lim) == 0)
		printf("Old limits -> soft limit= %ld \t"
		" hard limit= %ld \n", old_lim.rlim_cur,
							old_lim.rlim_max);
	else
		fprintf(stderr, "%s\n", strerror(errno));

	// Set new value
	lim.rlim_cur = 4;
	lim.rlim_max = 1024;

	
	// Set limits
	if(setrlimit(RLIMIT_NOFILE, &lim) == -1)
		fprintf(stderr, "%s\n", strerror(errno));
	
	// Get new limits
	if( getrlimit(RLIMIT_NOFILE, &new_lim) == 0)
		printf("New limits -> soft limit= %ld \t"
		" hard limit= %ld \n", new_lim.rlim_cur,
								new_lim.rlim_max);
	else
		fprintf(stderr, "%s\n", strerror(errno));
	
	// Try to open a new file
	// if(open("foo.txt", O_WRONLY | O_CREAT, 0) == -1)
	// 	fprintf(stderr, "%s\n", strerror(errno));
	// else
	// 		printf("Opened successfully\n");
	FILE *f=fopen("a.txt","r");
    if(f==NULL){
        printf("error\n");
    }
    else{
        printf("no error\n");
    }
	FILE *f2=fopen("a.txt","r");
    if(f2==NULL){
        printf("error\n");
    }
    else{
        printf("no error\n");
    }
	return 0;
}
