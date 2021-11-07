Name: Aditya Agarwal 
Roll No: 2019CSB1064
Course: CSL303
==================================================================================

1. What does the program do?

This program is used to simulate read-write processes by creating an initial queue
and then fulfilling the requests according the the given algorithm.

==================================================================================

2. Description 

First, the program takes the input through CLI. Then, it creates a queue of the requests
according to the given parameters. This is done in the generate_queue() function.

Second, the program calls the process() function to call the required algorithm.
Then, the requests are processed according to the given algorithm.

For each algorithm, the right process is selected in accordance to the given algorithm and
the algorithm then goes to the required cylinder.
While going to the required cylinder, there will also be some rotation of the disk, due to which
the current sector changes. This is also accounted for.
When the required cylinder is reached, there will be some rotational delay to get to the required sector
and additional delay to read from all the sectors.
seek delay + rotational delay + read delay = total delay.
Once the required sectors are read, the next request is processed.

In my program, the platter numbers are irrelevant as there is only one actuator arm, thus only
one platter's request will be processed at a time.

The scheduling algorithms considered are:
    1. Random
    2. FIFO
    3. SSTF
    4. SCAN
    5. C-SCAN

1. Random:
    It selects a random request from the queue and processes it.

2. FIFO:
    It selects the first request from the queue and processes it.

3. SSTF:
    It selects the request with the shortest distance from the current cylinder and processes it.

4. SCAN:
    It is like an elevator. It goes in one direction, and completes all the requests that it
    encounters, and then goes in the other direction and does the same.

5. C-SCAN:
    Similar to SCAN, but it goes in only one direction. It completes all the requests that it
    encounters while traversing the disk, and then it resets its head to the starting position.

CLI arguments needed are:
./a.out <r> <Ts> <N> <requests> <type>


Example:
./a.out 7500 4 512 1000 3

In this example, there is a rotation speed of 7500 rpm, average seek time of 4 milliseconds, sector size of 512 bytes,
number of requests is 1000, and the algorithm is 3(SSTF).



FIFO and Random_scheduling_algorithm perform much similar.
SSTF, SCAN and CSCAN also performs much similar.

SCAN had the highest throughput, followed by C-SCAN.
Thus SCAN was the best algorithm.
FIFO had the least throughput.

Overall this was observed:
SCAN> CSCAN>SSTF> Random> FIFO

==================================================================================

3. How to run:

    gcc disk.c -lm
    ./a.out <r> <Ts> <N> <requests> <type>

    Here all the values should be positive integers.
    
    Example:
    ./a.out 7500 4 512 1000 3