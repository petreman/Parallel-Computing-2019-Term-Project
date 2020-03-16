/**
 * AUCSC 310/450
 * Combined Term Project
 * Odd-Even Transposition Sort
 * 
 * oet_data.c
 * 
 * Serial and parallel data level implementation of odd-even transposition sort with PThreads
 * 
 * Generates an array of ints into memory based on the arguments from the command line. 
 * Then odd-even transpostion sort is used to serially or parallely sort the array
 * 
 * Implementation is data level because the sorting threads have all the data in the array
 * split up among them: there are no other task going on at the same time
 * 
 * The time to sort is recorded and printed to stdout
 * 
 * This implementation has more global variables than task level implementation.
 * This should be changed, but it doesn't really matter for something this trivial
 * 
 * Methods:
 *  - main(int argc, const char* argv[]) -> int
 *      Creates a globally avaliable array and executes the sort serially
 *      or paralelly based on args
 *  
 *  - serialOddEven(int arraySize) -> void
 *      Serial implementation of odd-even transposition sort.
 *      Operates on a globally avaliable array
 * 
 *  - parallelOddEven(int arraySize) -> void
 *      Parallel implementation of odd-even transposition sort using Pthreads.
 *      Operates on a globlly avaliable array, and creates array size / 2 threads
 * 
 *  - oddEvenStep(void *arg) -> void*
 *      The work each thread in the parallel implementation must do
 * 
 *  - swap(int x, int y) -> void
 *      Swaps the elements at indices x and y in the global array
 * 
 *  - printArray(int* array, int size) -> void
 *      Prints an array on a single line
 * 
 *  - Usage(const char* prog_name) -> void
 *      Prints to stderr how to use the program (its calling format)
 * 
 * 
 * Resources:
 *  - https://github.com/Nalaka1693/pthread_odd_even_sort/blob/master/odd-even-sort.c
 *      I found this guys implementation, which helped me structure my implementation quite
 *      a bit. Especially for how the threads will work, I thought I'd be stuck with
 *      array size / 2 threads (too much overhead)
 * 
 * Started November 13, 2019
 * Completed November 14, 2019
 * 
 * Keegan Petreman (petreman@ualberta.ca)
 * 1528679
*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

//Constants
#define MAX 1000 //set the upper bound for numbers generated

//Function Prototypes
void serialOddEven(int arraySize);
void parallelOddEven(int arraySize);
void* oddEvenStep(void* arg);
void swap(int x, int y);
void printArray(int* array, int size);
void Usage(const char* prog_name);

//Global Variables
int thread_count;
int* array;
pthread_barrier_t barrier;
pthread_mutex_t mutex;
bool global_swapped = true;
double elapsed = 0;

//struct: data for each thread
typedef struct {
    int myStart;
    int myEnd;
    int arraySize;
} thread_data;

/**
 * Preps the call to odd-even transpostion sort by first checking arguments,
 * and generating an array based on what size was provided. Then execution is
 * done serially or parallely based on args
 * 
 * @param argc: number of arguments given
 * @param argv[]: the arguments given
 * @return int
 */ 
int main(int argc, const char* argv[]){

    int arraySize;
    bool parallel = true;
    struct timespec stop, start;

    //check arguments
    switch (argc){

        case 1:
            arraySize = 8;
            thread_count = 2;
            break;

        case 2:
            
            if (0 == strcmp(argv[1], "-s")){
                parallel = false;
                arraySize = 8;
                thread_count = 1;
            }//if

            else{
                //get size of array from command line
                arraySize = strtol(argv[1], NULL, 10);
                thread_count = 2;
            }//else

            break;

        case 3:
            
            if (0 == strcmp(argv[1], "-s")){
                parallel = false;
                arraySize = strtol(argv[2], NULL, 10);
                thread_count = 1;
            }//if

            else{
                //get size of array from command line
                arraySize = strtol(argv[1], NULL, 10);
                thread_count = strtol(argv[2], NULL, 10);
            }//else

            break;   

        default:
            Usage(argv[0]);
            return EXIT_SUCCESS;   

    }//switch

    if (arraySize % thread_count != 0){
        fprintf(stderr, "Please only use array sizes that" 
        "can easily be divided by the thread count (defualt is %d)\n", thread_count);
        return EXIT_SUCCESS;
    }//if

    array = malloc(sizeof(int) * arraySize);

    srand((unsigned) time(NULL));

    //fill the array with random ints between 0 and MAX
    for (int i = 0 ; i < arraySize ; i++){
        array[i] =  (double)rand() / (double)(RAND_MAX / MAX);
    }//for

    if (parallel && thread_count > 1){
        
        parallelOddEven(arraySize);

        printf("\nParallel time on array of size %d (%d threads):\n"
            "%f seconds\n", arraySize, 
            thread_count, elapsed);

    }//if

    else{

        clock_gettime(CLOCK_MONOTONIC, &start);
        serialOddEven(arraySize);
        clock_gettime(CLOCK_MONOTONIC, &stop);

        elapsed = (stop.tv_sec - start.tv_sec);
        elapsed += (stop.tv_nsec - start.tv_nsec) / 1000000000.0;

        printf("\nSerial time on array of size %d:\n"
            "%f seconds\n", arraySize, elapsed);

    }//else

    free(array); 

    return EXIT_SUCCESS;

}//main

/**
 * Serial implementation of odd-even transposition sort
 * Pretty straight forward as this was heavily discussed in class
 * 
 * The number of phases is determined by n; the size of the array. 
 * But if array is sorted before n phases, exit early
 * 
 * @param arraySize: size of the array to be sorted. Used to determine stop case
 * @return void
 */ 
void serialOddEven(int arraySize){
    
    bool swapped;

    for (int phase = 0 ; phase < arraySize ; phase++){

        swapped = false;

        switch (phase % 2){

            //even phase
            case 0:
                
                for(int i = 0 ; i < arraySize ; i+=2){
                    if ( array[i] > array[i+1] ){
                        swap(i, i+1);
                        swapped = true;
                    }//if;
                }//for

                break;

            //odd phase
            case 1:

                for(int i = 1 ; i < arraySize - 1 ; i+=2){
                    if ( array[i] > array[i+1] ){
                        swap(i, i+1);
                        swapped = true;
                    }//if;
                }//for

                break;

        }//switch

        if (swapped = false){
            break;
        }//if

    }//for

}//serialOddEven

/**
 * Controller for the implementation of odd-even transposition sort.
 * Initialises a barrier for the threads, and creates all of them. Each thread
 * get the information it needs to find its chunk to sort
 * 
 * @param arraySize: size of the array to be sorted. Used to determine chunk size
 * @return void
 */ 
void parallelOddEven(int arraySize){
    
    pthread_t* thread_handles;
    int myLeft;
    int chunk = arraySize/thread_count;
    
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    pthread_barrier_init(&barrier, NULL, thread_count);

    for (int i = 0 ; i < thread_count ; i++){
        thread_data *my_data = (thread_data *) malloc(sizeof(thread_data));
        my_data->myStart = i * chunk;
        my_data->myEnd = (my_data->myStart) + chunk;
        my_data->arraySize = arraySize; 
        pthread_create(&thread_handles[i], NULL, oddEvenStep, (void *) my_data);
    }//for

    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }//for

    pthread_barrier_destroy(&barrier);

}//parallelOddEven

/**
 * Pthread Function
 * 
 * What every thread executes. This is where the sorting happens for the parallel implementation
 * 
 * Barriers are used to ensure all the threads enter the correct phase
 * at the same time. A global "global_swapped" boolean is used to exit the loop:
 * if none of the threads perform any swaps, then the array is sorted.
 * 
 * @param *arg: void pointer to the data the thread needs to begin its sort
 * @return void*
 */ 
void* oddEvenStep(void *arg){
    
    int myStart = ((thread_data *) arg)->myStart;
    int myEnd = ((thread_data *) arg)->myEnd;
    int arraySize = ((thread_data *) arg)->arraySize;
    bool swapped;
    
    //struct timeval stop, start, my_elapsed;
    struct timespec start, finish;
    double my_elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (global_swapped){

        pthread_barrier_wait(&barrier);
        swapped = false;
        global_swapped = false;

        for (int i = myStart ; i <= myEnd ; i += 2){

            //even phase
            if (array[i] > array[i + 1] ){
                swap(i, i + 1);
                swapped = true;
            }//if

        }//for

        pthread_barrier_wait(&barrier);

        //odd phase
        for (int i = myStart + 1; i <= myEnd - 1; i += 2) {
            
            if ( i + 1 < arraySize && array[i] > array[i + 1] ){
                swap(i, i + 1);
                swapped = true;
            }//if

        }//for   

        if (swapped && global_swapped == false){
            pthread_mutex_lock(&mutex);
            global_swapped = true;
            pthread_mutex_unlock(&mutex);
        }//if

        pthread_barrier_wait(&barrier);

    }//while

    clock_gettime(CLOCK_MONOTONIC, &finish);

    my_elapsed = (finish.tv_sec - start.tv_sec);
    my_elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    pthread_mutex_lock(&mutex);
    if (my_elapsed > elapsed){
        elapsed = my_elapsed;
    }//if
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(NULL);

}//oddEvenStep

/**
 * Swaps the two elements located at the provided indices of the array.
 * The array to be sorted is assumed to be of double precision numbers
 * 
 * @param array: pointer to the array of ints where the swap is to occur
 * @param x: first indice of array that needs to be swapped
 * @param y: second indice of array that needs to be swapped
 * @return void
 */  
void swap(int x, int y) {
   
   int temp;
   temp = array[x];
   array[x] = array[y];
   array[y] = temp;

}//swap

/**
 * Prints an array to stdout on a single line
 * 
 * Used in an eariler implementation
 * 
 * @param array: pointer to the array to print
 * @param size: the size of the array tp print
 * @return void
 */
void printArray(int* array, int size){

    for (int i = 0 ; i < size ; i++){
        printf ("%d ", array[i]);
    }//for

    printf("\n");

}//printArray

/**
 * Displays how to use the program.
 * I saw that Peter Pacheco used a similar function for his programs,
 * so I thought I'd add one for mine
 * 
 * @param prog_name: name of the program
 * @return void
 */ 
void Usage(const char* prog_name) {
   fprintf(stderr, "usage:   %s <-s> <n> <t>\n", prog_name);
   fprintf(stderr, "  's':  run serial odd-even transpostion sort\n");
   fprintf(stderr, "   n:   number of elements in list\n");
   fprintf(stderr, "   t:   number of threads to use\n");
}//Usage
