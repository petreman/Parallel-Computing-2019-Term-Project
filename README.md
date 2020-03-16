# Algorithm Design and Analysis / Parallel and Distributed Computing 2019 Combined Term Project: Odd-Even Transposition Sort

Odd-even transposition sort is a simple sorting algorithm used primarily on parallel processors. It’s a variant of bubble sort, an O(n^2) time complexity algorithm; but could the parallelization of odd-even transposition sort allow for faster execution than this? By creating a parallel implementation of the algorithm in C using PThreads, it’ll be possible to see how speedup and efficiency changes compared to the sequential implementation. But to see how it stands against other sorting algorithms, it’ll be compared to quicksort, a serial sorting algorithm that runs in O(nlgn) in the average case.

This is easily the most code I've ever written for a single project. This project should give good insight into my overall coding style.

## Files
- `oets_data.c`

   Serial and parallel data level implementation of odd-even transposition sort with PThreads. Generates an array of ints into memory based on the arguments from the command line. Then odd-even transposition sort is used to serially or parallelly sort the array. 
   
   Implementation is data level because the sorting threads have all the data in the array split up among them: there are no other task going on at the same time.

- `oets_task.c`

   Implementation of odd-even transposition sort with Pthreads

   Reads in 8 files of 100000 doubles each and sorts them using odd-even transposition. If the files, don't exist, they are created and filled first.

   Implementation is task level parallelism; even though the sorting threads have all the data in the array split up among them, other tasks are happening in the background to ensure the final result is calculated as quickly as possible (threads reading in more files and merging sorted files while other threads sort).

- `qs_data.c`

  Serial implementation of quicksort used for comparison of parallel odd-even transposition sort with data level parallelism. Generates an array of ints into memory based on arguments from the command line. Then quicksort is used to serially sort the array.
  
  There is no data level parallelism here; this program is simply used to compare with `oets_data.c`.

- `qs_task.c`

  Serial implementation of quicksort used for comparison of parallel odd-even transposition sort with task level parallelism. Reads into memory 8 files of 100000 doubles each, and sorts them using qsort().

  There is no task level parallelism here, but in odd-even sort there is; by having files brought into memory by a thread while other threads sort what's already available (and merging results when applicable). Both programs do the same thing, just differently. Meant for comparison with `oets_task.c`.

## Project Components:
1. [Proposal](https://ualbertaca-my.sharepoint.com/:w:/g/personal/petreman_ualberta_ca/EXjBLQkt6TZBhI-h6Fz8NXMBx6Mujh_67nV2bS4vx1UZlQ?e=1tz3T2) 

2. [Progress Report](https://ualbertaca-my.sharepoint.com/:w:/g/personal/petreman_ualberta_ca/EQa-jVmBBQBHozArgrNWHj0B0AUrfN1tApIdG1eIfJfe7A?e=B5EkuK)

3. [Results Presentation](https://ualbertaca-my.sharepoint.com/:p:/g/personal/petreman_ualberta_ca/EWu4dwkvWtVPn0QrP0Q1XRMBUHlmI85p6v5ygLxfaWlhFw?e=5qb76y)
  
   This presentation was given by me to the parallel computing class. I explained how odd-even transposition sort worked, its complexity, what I wanted to discover and how I approached the problem, and the results I had found.
   
4. [Formal Report](https://ualbertaca-my.sharepoint.com/:w:/g/personal/petreman_ualberta_ca/ER2PEH86ycdJuVEuoy0W3RkB9euUnQD59DlNSufG8RoZxw?e=nOf7di)

   Written report of methodology and findings, including a results discussion and overall conclusion: 
      
    > Odd-even transposition sort benefits greatly from being parallelized, especially when other tasks can run concurrently. However, it still can’t hold up against serial quicksort, as determined from testing.
