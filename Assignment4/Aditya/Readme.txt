Name: Het Fadia
Roll No: 2019CSB1084
Course: CSL303 Operating Systems
Assignment: 4

1. What does the program do?

Here requests are created in the program and then they are simulated according to the given algorithm.
Along with that the statistics are also calculated.


2. Description of proram:

The program takes type of algorithm, queue_size,number of requests etc as the input.

First it enqueues the requests in the queue.
Then according to the specified algorithm it does the process.

Here the following algorithm are implemented according to the types:
    0. Random_scheduling_algorithm
    1. FIFO
    2. SSTF
    3. SCAN
    4. CSCAN

0. Random_scheduling_algorithm:
It randomly selects a request from the queue and it is dequeued.

1. FIFO:
It dequeues the first inserted request from the queue and it is processed.

2. SSTF:
It dequeues the request with the least distance from the head of the queue and it is processed.

3. SCAN:
It moves from the start to the right and processes the requests. After that it goes from the end to the left and processes the requests.

4. CSCAN:
It moves from the start to the right and processes the requests. After that it goes from the left to the start position and processes the requests.

We can run the program with the following CLI command:
gcc main.c -lm
./a.out <r> <N> <Ts> <requests> <algorithm>
Here algorithm is between 0 to 4 (both inclusive) and according to it the algorithm is implemented
    0. Random_scheduling_algorithm
    1. FIFO
    2. SSTF
    3. SCAN
    4. CSCAN
r is RPM of the disk.
N is the size of sector in bytes.
Ts is the avg seeking time in ms.
requests is the number of requests.
Example:
gcc main.c -lm
./a.out 7500 512 4 1000 3

The program also prints the statistics of the program(the throughput etc)


FIFO and Random_scheduling_algorithm performs similar. 
SSTF, SCAN and CSCAN also perform similar and they perform much better than FIFO and Random_scheduling_algorithm.


SCAN had the highest throughput, followed by C-SCAN.
Thus SCAN was the best algorithm.
FIFO had the least throughput.

Overall this was observed:
SCAN> CSCAN>SSTF> Random> FIFO



3. How to run:

gcc main.c -lm
./a.out <r> <N> <Ts> <requests> <algorithm>
algorithm is between 0 to 4 (both inclusive) and according to it the type of the algorithm is implemented.
    0. Random_scheduling_algorithm
    1. FIFO
    2. SSTF
    3. SCAN
    4. CSCAN
r is RPM of the disk.
N is the size of sector in bytes.
Ts is the avg seeking time in ms.
requests is the number of requests.

Example:
gcc main.c -lm
./a.out 7500 512 4 1000 3
