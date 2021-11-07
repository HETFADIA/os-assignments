Name: Aditya Agarwal 
Roll No: 2019CSB1064
Course: CSL303
==================================================================================

1. What does the program do?

This program is used to simulate  bunch of processes requesting memory space according
to the required constraints, and the space is then allocated to the space depending
on the type of space partitioning algorithm used.

==================================================================================

2. Description 

When provided with the time duration and memory space constraints, multiple processes are
created at the required arrival rate and they request memory and time duration corresponding
to the constraints provided, randomly. 

These constraints, along with the total time duration of the simulation and the type of space
partitioning algorithm is given through the CL arguments.

There are 3 types of space partitioning algorithms in use:
    1. First-fit
    2. Best-fit
    3. Next-fit

The number corresponds to the type of algorithm.

CLI arguments needed are:
./a.out <p> <q> <n> <m> <t> <Total time duration of the program(m)> <Type of algorithm>


Example:
./a.out 1000 200 10 10 10 15 2

Value of p, q, n, m, and t are 1000, 200, 10, 10, and 10 respectively.
The simulation will run for a total duration of 15 minutes.
The type of algorithm to use is 2. Best-fit.

The experimental results as required are attached in the accompanying pdf file.
For each case, the algorithms are presented in order.

We can see that overall, best-fit performs the best in general. The results
were kind of mixed overall, however, best-fit did in general seem to perform 
better than the other two.

There can be a maximum of 10 processes in the queue to ensure that the turnaround time
is stable and doesn't go into infinity(which can happen if we take an infinite queue size).

==================================================================================

3. How to run:

    gcc memory.c -lpthread
    ./a.out <p> <q> <n> <m> <t> <Total time duration of the program(m)> <Type of algorithm>

    Here all the values should be positive integers.
    
    Example:
    ./a.out 1000 200 10 10 10 15 2