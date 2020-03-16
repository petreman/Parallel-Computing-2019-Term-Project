/**
 * AUCSC 310/450
 * Combined Term Project
 * Odd-Even Transposition Sort
 * 
 * oets_task.c
 * 
 * Implementation of odd-even transposition sort with Pthreads
 * Reads in 8 files of 100000 doubles each, and sorts them using odd-even transposition
 * 
 * Implementation is task level parallelism; even though the sorting threads have 
 * all the data in the array split up among them, other tasks are happening in the background to 
 * ensure the final result is calculated as quickly as possible (threads reading in more files 
 * and merging sorted files while other threads sort)
 * 
 * The time to sort is recorded and printed to stdout
 * 
 * Methods:
 *  - main(int argc, const char* argv[]) -> int
 *      Creates or opens the files needed, then calls odd-even sort serially
 *      or paralelly based on args
 * 
 *  - serialOddEven(double* array, int size) -> void
 *      Serial implementation of odd-even transposition sort
 *      Operates by first reading in all files into memory, then sorting with one thread
 * 
 *  - parallelOddEven(double* array) -> void
 *      Parallel implementation of odd-even transposition sort using Pthreads.
 *      Takes in the specified number of files of double numbers, and sorts them
 *      by having the user specified number of threads sort ach file, while 
 *      another 2 threads read in the next file and two sorted files if possible
 * 
 *  - openFiles() -> void
 *      Opens or creates all the files of doubles to be sorted
 * 
 *  - swap(double* array, int x, int y) -> void
 *      Swaps the elements at indices x and y in the provided array of doubles
 * 
 *  - readIn(void* rank) -> void*
 *      Pthread function
 *      Reads a file into memory
 * 
 *  - oddEvenStep(void *arg) -> void*
 *      Pthread function
 *      The work each sorting thread in the parallel implementation does
 * 
 *  - merge(void *args) -> void*
 *      Pthread function
 *      Merges two sorted subarrays
 * 
 *  - writeResult(double* array, const char* fileName) -> void
 *      Writes an array to file.
 *      Intended to be used after all files merged and sorted to get final result
 *  
 *  - startFetchThread(pthread_t* thread, int j) -> void
 *      Starts the fetch thread with its needed arguments to read in a file
 * 
 *  - startMergeThread(pthread_t *thread, int j) -> void
 *      Starts the merge with its needed arguments to merge two sorted subarrays
 * 
 *  - printArray(int* array, int size) -> void
 *      Prints an array on a single line to stdout
 * 
 *  - Usage(const char* prog_name) -> void
 *      Prints to stderr how to use the program
 * 
 * Resources:
 *  - https://github.com/Nalaka1693/pthread_odd_even_sort/blob/master/odd-even-sort.c
 *      I found this guys implementation, which helped me structure my implementation quite
 *      a bit. Especially for how the threads will work, I thought I'd be stuck with
 *      array size / 2 threads (too much overhead)
 *  
 *  - https://www.geeksforgeeks.org/merge-sort/
 *      Followed their algorithm for merging two sorted subarrays
 * 
 * Started November 13, 2019
 * Completed November 24, 2019
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
#define MAX 100000 //upper bound on the numbers generated
#define TOTAL_FILES 8 //how many files to sort
#define NUMS_PER_FILE 100000 //how many numbers in each file

//Function Prototypes
void serialOddEven(double* array, int size);
void parallelOddEven(double* array);
void openFiles();
void swap(double* array, int x, int y);
void* readIn(void* rank);
void* oddEvenStep(void *arg);
void* merge(void *args);
void writeResult(double* array, const char* fileName);
void startFetchThread(pthread_t* thread, double* array, int j);
void startMergeThread(pthread_t *thread, double* array, int j);
void printArray(double* array, int size);
void Usage(const char* prog_name);

//Global Variables
int thread_count;
FILE *fps[TOTAL_FILES];
pthread_barrier_t barrier;
pthread_mutex_t mutex;
bool global_swapped;

//Structs
//data for the fetch thread
typedef struct {
    double* array;
    int rank;
} fetch_thread_data;

//data for each sorting thread
typedef struct {
    double* array;
    int myStart;
    int myEnd;
    int endOfFile;
} sort_thread_data;

//data for the merging thread
typedef struct {
    double* array;
    int left;
    int mid;
    int right;
} merge_thread_data;

/**
 * Preps the call to odd-even transpostion sort by checking the arguments for serial 
 * or parallel execution, as well as generating the necessary files of data if they 
 * don't exist
 * 
 * @param argc: number of arguments given
 * @param argv[]: the arguments given
 * @return int
 */ 
int main(int argc, const char* argv[]){

    double* array;
    double elapsed;
    int arraySize = TOTAL_FILES * NUMS_PER_FILE;
    bool parallel = true;
    struct timespec stop, start;

    //check arguments
    switch (argc){

        case 1:
            thread_count = 2;
            break;

        case 2:
            
            if (0 == strcmp(argv[1], "-s")){
                parallel = false;
                thread_count = 1;
            }//if

            else{
                //get number of threads
                thread_count = strtol(argv[1], NULL, 10);
            }//else

            break;

        default:
            Usage(argv[0]);
            return EXIT_SUCCESS;   

    }//switch

    if (NUMS_PER_FILE % thread_count != 0){
        fprintf(stderr, "Please only use thread counts that" 
        "can easily divide the total numbers per file (%d)\n", NUMS_PER_FILE);
        return EXIT_SUCCESS;
    }//if

    srand((unsigned) time(NULL));

    //open/create the files
    openFiles();
    
    //allocate space in memory for numbers to be brought in
    array = malloc(sizeof(double) * arraySize);
    
    //parallel odd-even
    if (parallel && thread_count > 1){
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        parallelOddEven(array);
        clock_gettime(CLOCK_MONOTONIC, &stop);

        elapsed = (stop.tv_sec - start.tv_sec);
        elapsed += (stop.tv_nsec - start.tv_nsec) / 1000000000.0;

        printf("\nParallel time to sort %d files with %d numbers each (%d threads):\n"
            "%f seconds\n", TOTAL_FILES, NUMS_PER_FILE, thread_count, elapsed);

    }//if

    //serial odd-even
    else{
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        serialOddEven(array, arraySize);
        clock_gettime(CLOCK_MONOTONIC, &stop);

        elapsed = (stop.tv_sec - start.tv_sec);
        elapsed += (stop.tv_nsec - start.tv_nsec) / 1000000000.0;

        printf("\nSerial time to sort %d files with %d numbers each:\n"
            "%f seconds\n", TOTAL_FILES, NUMS_PER_FILE, elapsed);

    }//else

    free(array); 

    for (int i = 0 ; i < TOTAL_FILES ; i++){
        fclose(fps[i]);
    }//for

    return EXIT_SUCCESS;

}//main

/**
 * Serial implementation of odd-even transposition sort
 * A single thread reads the contents of all the files into memory, and the same
 * thread then sorts the combined files with odd-even transposition sort serially
 * 
 * Results of the sort are then written to a file
 * 
 * @param array: pointer to the array of doubles to be sorted
 * @param arraySize: size of the array to be sorted. Used to determine stop case
 * @return void
 */ 
void serialOddEven(double* array, int arraySize){
    
    bool swapped;

    //read in all the files at once into array in memory
    for (int i = 0 ; i < TOTAL_FILES ; i++){
        
        for (int j = 0 ; j < NUMS_PER_FILE ; j++){
            fscanf(fps[i], "%lf", &array[i * NUMS_PER_FILE + j]);
        }//for
        
    }//for

    for (int phase = 0 ; phase < arraySize ; phase++){

        swapped = false;

        switch (phase % 2){

            //even phase
            case 0:
                
                for(int i = 0 ; i < arraySize ; i+=2){
                    if ( array[i] > array[i+1] ){
                        swap(array, i, i+1);
                        swapped = true;
                    }//if;
                }//for

                break;

            //odd phase
            case 1:

                for(int i = 1 ; i < arraySize - 1 ; i+=2){
                    if ( array[i] > array[i+1] ){
                        swap(array, i, i+1);
                        swapped = true;
                    }//if;
                }//for

                break;

        }//switch

        if (swapped == false){
            break;
        }//if

    }//for

    //write back the result
    writeResult(array, "serialOetsResult.txt");

}//serialOddEven

/**
 * Controller for the implementation of parallel odd-even ransposition sort.
 * Initialises a barrier for the sorting threads, and creates all of them. Each thread
 * get the information it needs to find its even and odd pair for every step.
 * 
 * Number of threads to be sorting is specified by the user
 * One thread reads in the files, and another merges two sorted subarrays
 * 
 * After execution finishes, the results are written to a file
 * 
 * @param array: pointer to the array of doubles to be sorted
 * @return void
 */ 
void parallelOddEven(double* array){
    
    pthread_t* thread_handles;
    pthread_t fetch_thread, merge_thread;
    int fileStart;
    int chunk = NUMS_PER_FILE/thread_count; //how many numbers for each thread to sort
    
    //allocate the sorting threads, total specified by user
    thread_handles = malloc(thread_count * sizeof(pthread_t));

    pthread_barrier_init(&barrier, NULL, thread_count);

    //need to read in first file to begin sorting and wait for it 
    //to join to make sure that there is correct data to sort
    startFetchThread(&fetch_thread, array, 0);
    pthread_join(fetch_thread, NULL);

    //loop to get every file
    for (int j = 0 ; j < TOTAL_FILES ; j++){

        //get beginning index in array for current file
        fileStart = (j * NUMS_PER_FILE);

        //make sure next read in is finished before calling sort
        if (j > 0){
            pthread_join(fetch_thread, NULL);
        }//else
        
        //start sorting file
        //
        //I could and should of put everything in here into a function, but then 
        //i, chunk, and fileStart would all have to be passed as parameters
        for (int i = 0 ; i < thread_count ; i++){
            
            sort_thread_data *my_sort_data = 
                (sort_thread_data *) malloc(sizeof(sort_thread_data));

            if (my_sort_data == NULL) {
                fprintf(stderr, "Couldn't allocate memory for thread arg\n");
                exit(EXIT_FAILURE);
            }//if

            my_sort_data->array = array;
            my_sort_data->myStart = (i * chunk) + fileStart;
            my_sort_data->myEnd = (my_sort_data->myStart) + chunk;
            my_sort_data->endOfFile = fileStart + (NUMS_PER_FILE - 1);
            pthread_create(&thread_handles[i], NULL, oddEvenStep, (void *) my_sort_data);

        }//for

        //join merge_thread before calling it again below
        if (j > 2){
            pthread_join(merge_thread, NULL);
        }//if
        
        //if 2 or more files have been brought in and sorted, merge them
        if (j > 1){
            startMergeThread(&merge_thread, array, j);
        }//if

        //read in the next file while previous is sorting
        if (j+1 < TOTAL_FILES){
            startFetchThread(&fetch_thread, array, j + 1);
        }//if

        //if no more files, join the fetch thread
        else{
            pthread_join(fetch_thread, NULL);
        }//else
        
        //join the sorting threads
        for (int i = 0; i < thread_count; i++) {
            pthread_join(thread_handles[i], NULL);
        }//for

    }//for   

    //Have to join the merge thread before calling it again to merge in the last file
    pthread_join(merge_thread, NULL);
    startMergeThread(&merge_thread, array, TOTAL_FILES);
    pthread_join(merge_thread, NULL);

    //after last merge finished, write result of sort to file
    writeResult(array, "paralllelOetsResult.txt");

    //cleanup
    free(thread_handles);
    pthread_barrier_destroy(&barrier);

}//parallelOddEven

/**
 * Opens or creates all the files of doubles to be sorted.
 * Their file pointers are stored in a globally accessible array
 * 
 * @return void
 */ 
void openFiles(){

    char filename[10];

    for (int i = 0 ; i < TOTAL_FILES ; i++){

        sprintf(filename, "data%d.txt", i+1);

        fps[i] = fopen(filename, "ab+");
        
        if (fps[0] == NULL){ 
            perror("Error"); 
            exit(EXIT_FAILURE);
        }//if

        /*
         * Check if each file is empty or not.
         * If it is, generates NUMS_PER_FILE random doubles 
         * for the file. Otherwise nothing happens 
         * (assumed it has correct amount of random doubles already)
         */ 

        // goto end of file
        if (fseek(fps[i], 0, SEEK_END) != 0){
            perror("Error"); 
            exit(EXIT_FAILURE);
        }//if
        
        //if seek to end didn't move pointer, file is empty
        if (ftell(fps[i]) == 0){
            
            printf("Filling file data%d.txt...\n", (i+1) );
            
            //fill file with doubles with range 0 to MAX
            for (int j = 0 ; j < NUMS_PER_FILE ; j++){
                fprintf(fps[i], "%lf ", 
                    (double)rand() / (double)(RAND_MAX / MAX));
            }//for

        }//if

        //go back to beginning of file
        fseek(fps[i], 0, SEEK_SET);

    }//for

}//openFiles

/**
 * Swaps the two elements located at the provided indices of the array.
 * The array to be sorted is assumed to be of double precision numbers
 * 
 * @param array: pointer to the array of doubles where the swap is to occur
 * @param x: first indice of array that needs to be swapped
 * @param y: second indice of array that needs to be swapped
 * @return void
 */ 
void swap(double* array, int x, int y) {
   
   double temp;
   temp = array[x];
   array[x] = array[y];
   array[y] = temp;

}//swap

/**
 * Pthread Function
 * 
 * Reads a file into the array so it may be sorted afterwards in memory
 * 
 * @param *arg: pointer to the data the thread needs to read the file in
 * @return void*
 */ 
void* readIn(void *arg){
    
    double* array = ((fetch_thread_data *) arg)->array;
    int my_rank = ((fetch_thread_data *) arg)->rank;
    int offsetForFile = my_rank * NUMS_PER_FILE;

    free(arg);

    for (int offsetWithinFile = 0 ; offsetWithinFile < NUMS_PER_FILE ; offsetWithinFile++){
        fscanf(fps[my_rank], "%lf", &array[offsetForFile + offsetWithinFile]);
    }//for

    pthread_exit(NULL);

}//readIn

/**
 * Pthread Function
 * 
 * What every sorting thread executes. This is where the sorting happens for the parallel implementation
 * 
 * Barriers are used to ensure all the threads enter the correct phase
 * at the same time. A global "global_swapped" boolean is used to exit the loop:
 * if none of the threads perform any swaps, then the array is sorted.
 * 
 * @param *arg: pointer to the data the thread needs to begin its sort
 * @return void*
 */ 
void* oddEvenStep(void *arg){
    
    bool swapped; //local swap variable
    double* array = ((sort_thread_data *) arg)-> array;
    int myStart = ((sort_thread_data *) arg)->myStart;
    int myEnd = ((sort_thread_data *) arg)->myEnd;
    int endOfFile = ((sort_thread_data *) arg)->endOfFile;

    free(arg);

    //sort while globally (across all threads) is a swap that happens
    do {

        pthread_barrier_wait(&barrier);
        swapped = false;
        global_swapped = false;

        //even phase
        for (int i = myStart ; i <= myEnd ; i += 2){

            if (i + 1 <= endOfFile && array[i] > array[i + 1] ){
                swap(array, i, i + 1);
                swapped = true;
            }//if

        }//for

        pthread_barrier_wait(&barrier);

        //odd phase
        for (int i = myStart + 1; i <= myEnd - 1; i += 2) {
            
            if ( i + 1 < endOfFile && array[i] > array[i + 1] ){
                swap(array, i, i + 1);
                swapped = true;
            }//if

        }//for   

        if (swapped && (global_swapped == false)){
            pthread_mutex_lock(&mutex);
            global_swapped = true;
            pthread_mutex_unlock(&mutex);
        }//if

        pthread_barrier_wait(&barrier);

    } while (global_swapped);
    
    pthread_exit(NULL);

}//oddEvenStep

/**
 * Pthread function
 * 
 * Takes a pointer to an array of doubles where the subarrays 0 to mid is and mid+1 
 * to end is sorted, and combines them into one whole sorted array.
 * 
 * Adapted from my merge sort implementation from 310 
 * (which itself was taken from https://www.geeksforgeeks.org/merge-sort/)
 * 
 * @param *args: pointer to the struct of data the thread needs to merge
 * @return void*: pthread function
 */ 
void* merge(void *arg){

    //get args
    double* array = ((merge_thread_data *) arg)->array;
    int left = ((merge_thread_data *) arg)->left;
    int mid = ((merge_thread_data *) arg)->mid;
    int right = ((merge_thread_data *) arg)->right;    

    free(arg);

    int i, j, k; 
    int n1 = mid - left + 1; 
    int n2 =  right - mid; 
  
    //create temp arrays
    double* L = malloc(sizeof(double) * n1);
    double* R = malloc(sizeof(double) * n2);

    //Copy data to temp arrays L[] and R[]
    for (i = 0; i < n1; i++){
        L[i] = array[left + i];
    }//for 
         
    for (j = 0; j < n2; j++){
        R[j] = array[mid + 1 + j];
    }//for 
         
    //Merge the temp arrays back into arr[l..r]
    i = 0; // Initial index of first subarray 
    j = 0; // Initial index of second subarray 
    k = left; // Initial index of merged subarray 

    while (i < n1 && j < n2) { 
        
        if (L[i] < R[j]) { 
            array[k] = L[i]; 
            i++; 
        }//if 

        else { 
            array[k] = R[j]; 
            j++; 
        }//else

        k++; 

    }//while 
  
    //Copy the remaining elements of L[], if there are any
    while (i < n1) { 
        array[k] = L[i]; 
        i++; 
        k++; 
    }//while 
  
    //Copy the remaining elements of R[], if there are any
    while (j < n2) { 
        array[k] = R[j]; 
        j++; 
        k++; 
    }//while 

    free(L);
    free(R);

    pthread_exit(NULL);

}//merge

/**
 * Writes the provided array of doubles to the provided file
 * Amount of numbers to be written is based on how many were in the source file
 * 
 * @param array: pointer to the array to be written
 * @param filename: name of the file the array will be written to
 * @return void
 */ 
void writeResult(double* array, const char* filename){
    
    //create/truncate file to store results
    FILE *fp = fopen(filename, "w+");

    //fopen returns the NULL pointer on failure
    if (fp == NULL){ 
        perror("Error"); 
        exit(EXIT_FAILURE);
    }//if 

    //write back to new file
    for (int i = 0 ; i < TOTAL_FILES ; i++){

        int k = i * NUMS_PER_FILE;
        
        for (int j = 0 ; j < NUMS_PER_FILE ; j++){
            fprintf(fp, "%lf ", array[k + j]);
        }//for
        
    }//for

    fclose(fp);

}//writeResult

/**
 * Starts the thread responsible for bringing files into memory.
 * File to bring in is determined by rank
 * 
 * @param *thread: address of the thread to bring file into memory
 * @param array: the array to store the fetch to
 * @param rank: rank of the file to bring in (index into fps)
 * @return void
 */ 
void startFetchThread(pthread_t *thread, double* array, int rank){

    fetch_thread_data *fetch_data = 
        (fetch_thread_data *) malloc(sizeof(fetch_thread_data));
            
    if (fetch_data == NULL) {
        fprintf(stderr, "Couldn't allocate memory for thread arg\n");
        exit(EXIT_FAILURE);
    }//if

    fetch_data->array = array;
    fetch_data->rank = rank; 

    pthread_create(thread, NULL, readIn, (void*) fetch_data);

}//startFetchThread

/**
 * Starts the thread responsible for merging the sorted files j and j-1
 * 
 * @param *thread: address of the thread to do the merge
 * @param array: the array with the subarrays to merge
 * @param j: used to determine which files to merge (indexes into fps)
 * @return void
 */ 
void startMergeThread(pthread_t *thread, double* array, int j){
    
    merge_thread_data *merge_data = 
        (merge_thread_data *) malloc(sizeof(merge_thread_data));

    if (merge_data == NULL) {
        fprintf(stderr, "Couldn't allocate memory for thread arg\n");
        exit(EXIT_FAILURE);
    }//if

    merge_data->array = array;
    merge_data->left = 0;
    merge_data->mid = ( (j-1) * NUMS_PER_FILE ) - 1;
    merge_data->right = (j * NUMS_PER_FILE) - 1;    
    pthread_create(thread, NULL, merge, (void *) merge_data);

}//startMergeThread

/**
 * Prints an array to stdout on a single line
 * 
 * Used in an eariler implementation
 * 
 * @param array: pointer to the array to print
 * @param size: the size of the array tp print
 * @return void
 */
void printArray(double* array, int size){

    for (int i = 0 ; i < size ; i++){
        printf ("%lf ", array[i]);
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
   fprintf(stderr, "usage:   %s <-s/t>\n", prog_name);
   fprintf(stderr, "  's':  run serial odd-even transpostion sort\n");
   fprintf(stderr, "   t:   run parallel odd-even transpostion sort with t threads\n");
}//Usage
