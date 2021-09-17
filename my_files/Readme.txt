Name : Het Fadia
Roll Call: 2019CSB1084
Course : CSL303-Operating System  
Assignment 1

======================================================================
What does the program do?
We implemented a multithreaded server, a socket based server-client program which can process multiple clients at the same time using a multithreading library ie #include<pthread.h> in c.
The server takes the library name, function name, arguments from the client processes it and prints the output in the terminal.

The program allows multiple clients to send requests, their request can also be processed simultaneously. The dispatcher then handles the requests of the clients.

Moreover memory limit, thread limit, file open limit etc can be given.


A description of how the program works
The program basically makesa a socket, binds to it. After that the server processes the requests from multiple continuously and simultaneously in a multithreading environment.

The clients requests their requests in the following manner:
Dynamic loaded library(dll_name)
function name
function arguments
All these are in a string form separated by a “?”

The example of such a request is /lib/x86_64-linux-gnu/libm.so.6?tan?5
Here the dll name is /lib/x86_64-linux-gnu/libm.so.6
The function name is tan
Here the arguments is 5
Each of them is separated by “?”

The server then processes the string, loads the library and calls the function with the given arguments.
The output is then printed in the terminal and a message is sent to the client about the confirmation that the above process was successful.

The dll handler function enqueues the requests of the client and whenever a thread is available the function dequeues the requests and the request is processed.

Executing the files:

To execute the file download the main.c, client.c and input.txt

Executing the main.c

In the Terminal write
gcc main.c -lpthread -ldl -o main
./main.out [PORT] [Thread limit] [limit of the number of files that can be opened] [max_memory]

Here the memory is in kbs
Then in the new terminal
gcc client.c
./a.out [PORT]

The following constraints are there for better functioning of the server:
Thread limit>=1
No of files that can be opened >=6
Max memory >=6000

About the input.txt
In the input.txt the input each line consists of 
First the dll name, then a “?” Then the function name,”?” and then And finally the arguments of the function
The input file can have a maximum of 1000 requests.
Currently my program can execute only math function having arguments of type all double and the length of the arguments can be max 6

The input can be manually changed to include more requests.

The program allows the following functions to be executed
tan, sin, cos, acos, atan, sqrt, cbrt, floor, ceil,abs
pow
Many other mathematical functions having less than 6 arguments (all of them having return data type double as well as all the arguments should be double)
The program currently only executes mathematical functions


Testing
To test the program write the following command in the terminal
gcc main.c -lpthread -ldl
./a.out test [Thread limit] [limit of the number of files that can be opened] [max_memory]
