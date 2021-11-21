Submitter name: Shikhar Soni
Roll No.: 2019CSB1119
Course: CS303
=================================
1. What does this program do

This program creates a FUSE based file system that has the ability to be able to store its files in real 
time to a mail inbox whose credentials you have provided and allows for the user using the file system
the functionalities of a normal file system while storing the data of the files into the inbox of the mail ID.
The normal functionalities of files such as reading, writing, creating files/directories, etc. are handled by
the file system. 

2. A description of how this program works (i.e. its logic)

This program works by assuming a special mail by the subject "IMP" that has a specific format (can't
run without the right format, therefore it's essential to make sure that it's in the right format)
that contains the info about the various files and folders in the file system, this information is
updated as we add or delete files and provides a reference to the file structure that only supports
one level of depth of directory below the root directory.

The information from the mail is read first and the information is supplied to the file
system to get a clear idea of the file system and it's structure, names and emails associated
with them.

All the mails are stored in the inbox and are deleted/updated when someone writes or deletes some
content from them as the mails in inbox on gmail are read only. For instance when we want to write
we read the original mail, then we find and delete the mail and then send another mail in it's place
with the updated content. This way we make the inbox work like a file system.

The file system only allows files of size of 1024 and max 1024 files in a folder and a max of 10000 files.
The file system also requires a configuration file that passes all the login and connection details to
make a connection with the mail.

3. How to compile and run this program

To run this program:
install libfuse and libcurl libraries before trying to run the below commands.
1. for libfuse use -> sudo apt-get install libfuse-dev
2. for libcurl use -> sudo apt install libcurl4-openssl-dev

1. gcc fs.c -lcurl `pkg-config fuse --cflags --libs`
2. ./a.out [Configure file path] [Mount path]
E.g., ./a.out configure.txt SPEC_FOLDER

4. Provide a snapshot of a sample run
Refer to the jpeg present in the zipped folder for your reference.
