#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>


int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, NULL, NULL);
}