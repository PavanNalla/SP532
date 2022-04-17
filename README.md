CS532 Hw4

Working partners:
Pavan Kumar Nalla,
Sushmitha Vaggu

Demo done with alyssa on 04/15/2022


File name: Hw4

To compile:   gcc -o Hw4 Hw4.c -pthread

To execute: ./Hw4 2

Sample Output 1:


Enter Command> submit /home/UAB/pnalla/CS532/Final/hw1 2000
job 1 added to the queue
Enter Command> showjobs
jobid   command                                 status
1       /home/UAB/pnalla/CS532/Final/hw1 2000   working
Enter Command> submit /home/UAB/pnalla/CS532/Final/hw2 2000
job 2 added to the queue
Enter Command> showjobs
jobid   command                                 status
1       /home/UAB/pnalla/CS532/Final/hw1 2000   working
2       /home/UAB/pnalla/CS532/Final/hw2 2000   working
Enter Command> submit /home/UAB/pnalla/CS532/Final/hw3 2000
job 3 added to the queue
Enter Command> showjobs
jobid   command                                 status
1       /home/UAB/pnalla/CS532/Final/hw1 2000   working
2       /home/UAB/pnalla/CS532/Final/hw2 2000   working
3       /home/UAB/pnalla/CS532/Final/hw3 2000   waiting
Enter Command> submithistory

wait for two minutes as the waittime time is same (2 minutes)

jobid   command                                 starttime                       endtime
Enter Command> Time taken for size 2000 = 189.748278 seconds
Time taken for size 2000 = 197.482572 seconds

Enter Command> submithistory
jobid   command                                 starttime                       endtime
1       /home/UAB/pnalla/CS532/Final/hw1 2000   Sun Apr 17 16:56:15 2022        Sun Apr 17 16:59:25 2022        complete
2       /home/UAB/pnalla/CS532/Final/hw2 2000   Sun Apr 17 16:56:32 2022        Sun Apr 17 16:59:49 2022        complete
Enter Command> Time taken for size 2000 = 141.210072 seconds





