Name : Het Fadia
Roll Call: 2019CSB1084
Course : CSL303-Operating System  
Assignment 1


======================================================================
1. What does the program do?
We implemented a multithreaded server, a socket based server-client program which can process multiple clients at the same time using a multithreading library ie #include<pthread.h> in c.
The server takes the library name, function name, arguments from the client processes it and prints the output in the terminal.


The program allows multiple clients to send requests, their request can also be processed simultaneously. The dispatcher then handles the requests of the clients.
Due to multithreading the request of the client can be processed simultaneously so that he/she does not have to wait.


Moreover memory limit, thread limit, file open limit etc can be given.






2. A description of how the program works
The program basically makesa a socket, binds to it. After that the server processes the requests from multiple continuously and simultaneously in a multithreading environment.


The clients requests their requests in the following manner:
1. Dynamic loaded library(dll_name)
2. function name
3. function arguments
All these are in a string form separated by a “?”


The example of such a request is /lib/x86_64-linux-gnu/libm.so.6?tan?5
Here the dll name is /lib/x86_64-linux-gnu/libm.so.6
The function name is tan
Here the arguments is 5
Each of them is separated by “?”


The server then processes the string, loads the library and calls the function with the given arguments.
The output is then printed in the terminal and a message is sent to the client about the confirmation that the above process was successful.


The dll handler function enqueues the requests of the client and whenever a thread is available the function dequeues the requests and the request is processed.


Whenever the program tries to exceed the limits ie. memory limit, thread limit or file open limit, the program does not allow it.
In case of memory, the program does not allow to make more arrays until the memory frees.
Similarly in the case of files, no more files can be opened beyond the file open limit


3. Executing the files:


To execute the file download the main.c, client.c and input.txt


Executing the main.c


In the Terminal write
gcc main.c -lpthread -ldl -o main
./main [PORT] [Thread limit] [limit of the number of files that can be opened] [max_memory]
Eg ./main 7000 10 10 10000


Here the memory is in kbs
Then in the new terminal
gcc client.c
./a.out [PORT]
Eg. ./a.out 7000


The following constraints are there for better functioning of the server:
* Thread limit>=1
* No of files that can be opened >=6
* Max memory >=6000


4. About the input.txt
In the input.txt the input each line consists of 
First the dll name, then a “?” Then the function name,”?” and then And finally the arguments of the function
The input file can have a maximum of 1000 requests.
Currently my program can execute only math function having arguments of type all double and the length of the arguments can be max 6


The input can be manually changed to include more requests.


The program allows the following functions to be executed
* tan, sin, cos, acos, atan, sqrt, cbrt, floor, ceil,abs
* pow
* Many other mathematical functions having less than 6 arguments (all of them having return data type double as well as all the arguments should be double)
* The program currently only executes mathematical functions




5. Testing
To test the program write the following command in the terminal
gcc main.c -lpthread -ldl -o main
./main test [Thread limit] [limit of the number of files that can be opened] [max_memory]


Eg ./main 7000 10 10 10000


In the testing part, various checks have been done like passing the wrong function name, passing the wrong path.


I also tried to open more than open_file_limit files and the file could not be opened.


Most of the corner cases have been handled in the testing part.