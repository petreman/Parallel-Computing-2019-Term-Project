/**
 * AUCSC 310/450
 * Combined Term Project
 * Quicksort
 * 
 * qs_task.c
 * 
 * Serial implementation of quicksort used for comparsion of 
 * parallel odd-even transposition sort with task level parallelism
 * 
 * Reads into memory 8 files of 100000 doubles each, and sorts them using qsort()
 * 
 * There is no task level parallelism here, but in odd-even sort there is; by having files 
 * brought into memory by a thread while other threads sort what's already avaliable 
 * (and merging results when applicable). Both programs do the same thing, just differently
 * 
 * The time to sort is recorded and printed to stdout
 * 
 * Methods:
 *  - main(int argc, const char* argv[]) -> int
 *      Creates a globally avaliable array and executes the sort serially
 *      or paralelly based on args (only serial is implemented)
 * 
 *  - openFiles() -> void
 *      Opens or creates all the files of doubles to be sorted
 * 
 *  - readInFiles(double* array) -> void 
 *      Reads all the files into an array in memory
 * 
 *  - writeResult(double* array, const char* filename) -> void
 *      Writes an array to file.
 *      Intended to be used after all files merged and sorted to get final result
 * 
 *  - cmp(const void *x, const void *y) -> int
 *      Comparsion function for qsort(). Compares double precision numbers
 * 
 *  - Usage(const char* prog_name) -> void
 *      Prints to stderr how to use the program
 * 
 * Started November 13, 2019
 * Completed November 25, 2019
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

//Constants
#define MAX 100000 //upper bound on the numbers generated
#define TOTAL_FILES 8 //how many files to sort
#define NUMS_PER_FILE 100000 //how many numbers in each file

//Global Variables  
FILE *fps[TOTAL_FILES];
int thread_count;

//Function Prototypes
void openFiles();
void readInFiles(double* array);
void writeResult(double* array, const char* filename);
int cmp(const void *x, const void *y);
void Usage(const char* prog_name); 

/**
 * Preps the call to quicksort by checking the arguments for serial 
 * or parallel execution, as well as generating the necessary files of data if they 
 * don't exist.
 * 
 * Parallel sort is not implemented, please use serial sort
 * 
 * @param argc: number of arguments given
 * @param argv[]: the arguments given
 * @return int
 */
int main(int argc, const char* argv[]){ 
    
    double* array;
    double elapsed = 0;
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

    srand((unsigned) time(NULL));

    //try to open/create the files
    openFiles();

    array = malloc(sizeof(double) * arraySize);

    if (parallel && thread_count > 1){
        
        printf("Parallel quicksort not implemented, please use serial (\"-s\")\n");
        free(array);
        return EXIT_SUCCESS;

        //leave this here for when parallel implemented
        printf("\nParallel time to sort %d files with %d numbers each (%d threads):\n"
            "%f seconds\n", TOTAL_FILES, NUMS_PER_FILE, thread_count, elapsed);

    }//if

    else{
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        readInFiles(array);
        qsort(array, arraySize, sizeof(double), cmp);
        writeResult(array, "qsResult.txt");
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
 * Reads all the files into the array so it may be sorted afterwards in memory
 * 
 * @param array: pointer to the data the thread needs to read the file in
 * @return void
 */ 
void readInFiles(double* array){
    
    //read in all the files at once into array in memory
    for (int i = 0 ; i < TOTAL_FILES ; i++){
        
        for (int j = 0 ; j < NUMS_PER_FILE ; j++){
            fscanf(fps[i], "%lf", &array[i * NUMS_PER_FILE + j]);
        }//for
        
    }//for

}//readInFiles

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
 * Comparison function for qsort() 
 * For comparing doubles 
 * 
 * Taken from https://stackoverflow.com/a/8448818
 * 
 * @param *x: pointer to double in array
 * @param *y: pointer to double in array
 */ 
int cmp(const void *x, const void *y){
    double xx = *(double*)x, yy = *(double*)y;
    if (xx < yy) return -1;
    if (xx > yy) return  1;
    return 0;
}//cmp

/**
 * Displays how to use the program.
 * I saw that Peter Pacheco used a similar function for his programs,
 * so I thought I'd add one mine
 * 
 * @param prog_name: name of the program
 * @return void
 */ 
void Usage(const char* prog_name){
   fprintf(stderr, "usage:   %s <-s> <n>\n", prog_name);
   fprintf(stderr, "  's':  run serial quicksort sort\n");
   fprintf(stderr, "   t:   number of threads to use\n");
}//Usage
