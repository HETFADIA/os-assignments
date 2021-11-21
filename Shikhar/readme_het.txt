Name: Het Fadia
Entry No: 2019CSB1084

Course:CS303

1. What does this program do?

Our program creates fuse system that uses gmail to store,edit, read and write and delete files and folders.
Here we data of files and folders is stored in inbox of the email and the password which was provided by the user.
We are using here fuse and curl libraries to form a connection between email and the file system.
There are arrays that stores the names of the files, the data of the files and directories.
Thus this system is called SSTF(Simple yet stupid file system).

Our program takes credentials of the email, server ,password etc from a file named configure.txt


2. A description of how the program works:

This 

3. How to compile and run this program:

Unzip the folder.
Enter your email in the 5th line of configure.txt and the password of the email in the 6th line.
Then make a directory named store(or any other name)
Now run the below command in terminal.
gcc main.c `pkg-config fuse --cflags --libs` -lcurl
./a.out  -f store

Here store can be replaced by the path of the directoy you made

Now open the new terminal and execute the below command
cd store
Here we can use all the commands of the file system
some of the examples are listed below
1. ls
2. cd
3. cat bro.txt
4. ll
5. cd ..
6. cd folder
7. echo "hello" > file.txt