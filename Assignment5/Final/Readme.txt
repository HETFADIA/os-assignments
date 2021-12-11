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

Here in our program the commands gets executed as the user writes commands in the terminal.
For example as the user writes cat hi.txt the SS_read function will automatically get executed.
Thus the contents of the file will be printed in the terminal.
Similary when the user writes echo "hello" > hi.txt the SS_write function will automatically get executed.
First the hi.txt will be cleared and then the string will be copied in the file.

The mails gets stored in the inbox of the email. So whenever the user executes the commnads the content is taken from the email.



3. How to compile and run this program:

Unzip the folder.
Enter your email in the 5th line of configure.txt and the password of the email in the 6th line.
Also change the configure.txt according to you.
Then make a directory named store(or any other name)
Now run the below command in terminal.
gcc main.c `pkg-config fuse --cflags --libs` -lcurl
./a.out  store configure.txt



