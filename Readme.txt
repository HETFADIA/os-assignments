Name : Het Fadia
Entry No: 2019csb1084
Course: CS303 (Operating System) by Balwinder Sodhi
Assignment No:3

#####1 What does this program do :
Our program is multithreaded. There is a thread which keeps on enqueueing the processes.
Another thread makes the dequeueing part and runs the processes.
Each thread runs a single process.
Each process is of random time interval and takes random space.
When the process ends the memory is freed.
Our program runs for the total_run_time minutes and prints the memory percentage used, turnaround time and other details.

#####2 Description of the program:
Our objective is to investigate the relative effectiveness of the first-fit, best-fit, and next-fit algorithms for dynamic partitioning based memory placement.
Here we are measuring the effectiveness of the above algorithms by varing parameters like p,q,n,m,t.

The output printed in the console is described below:
The memory array chunk consists of 10 MB as the allocations is of 10MB sizes.
The memory array is printed:
arr[i]=1 means it is occupied with a processes.
arr[i]=0 means it is not occupied with a process.

For each dequeued process I have printed:
Duration time in seconds.
Memory it will take in MB
And finally the index.

The process with the memory and size enqueued is shown.

Also the number of processes executed are printed.
Along with that turnaround time and the Memory utilization is printed.
The program will run for total_run_time minutes.
The type of function can be 1 2 and 3.
Type=1 means first fit algo will be run.
Type=2 means next fit algo will be run
Type=3 means best fit algo will be run.

--Time for stabilization
The time at which most of the processes stabilized was around 13 minutes.

--Conclusion
We conclude that the first algo is the best as the turn around time is least.
And the Best fit algo is worst as the turn around time is the worst.

Thus the best algorithm is described below:
first fit>next fit>best fit algorithm

Due to this I would use the first fit algorithm.


#####3 How to compile and run the program :
First extract the folder:
The compile it using gcc main.c -lpthread
Next write ./a.out and the below arguments
The arguments should be p q n m t total_run_time and type of the function.

The format is below:

gcc main.c -lpthread
./a.out <p> <q> <n> <m> <t> <total_run_time> <type>

For example we can run the program using:

gcc main.c -lpthread
./a.out 1000 200 10 10 10 10 1