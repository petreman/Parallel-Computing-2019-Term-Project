/**
 * AUCSC 310/450
 * Combined Term Project
 * Quicksort
 * 
 * qs_data.c
 * 
 * Serial implementation of quicksort used for comparsion of 
 * parallel odd-even transposition sort with data level parallelism
 * 
 * Generates an array of ints into memory based on arguments from the command line. 
 * Then quicksort is used to serially sort the array
 * 
 * There is no data level parallelism here; this program is simply used to compare with 
 * odd-even transposition sort
 * 
 * The time to sort is recorded and printed to stdout
 * 
 * Methods:
 *  - main(int argc, const char* argv[]) -> int
 *      Creates a globally avaliable array and executes the sort serially
 *      or paralelly based on args (only serial is implemented)
 * 
 *  - partition (int* array, int low, int high) -> int
 *      High is the pivot, which is placed in the correct position of the array.
 *      Everything smaller is palce before it, and everything larger after it.
 *      Returns position index of pivot
 * 
 *  - quickSort(int* array, int low, int high) -> void
 *      Executes the sorting algorithm quicksort. Serial
 * 
 * - swap(int x, int y) -> void
 *      Swaps the elements at indices x and y in the global array
 * 
 *  - printArray(int* array, int size) -> void
 *      Prints an array on a single line
 * 
 *  - Usage(const char* prog_name) -> void
 *      Prints to stderr how to use the program
 * 
 * Resources:
 *  - https://www.geeksforgeeks.org/quick-sort/
 *      I pretty much copied their implementaion of quicksort. It's the same as
 *      was discussed in class, and I was more worried about implementing odd-even
 *      transposition sort
 * 
 * Started and completed November 13, 2019
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

//set the upper bound for numbers generated
#define MAX 1000

//global variables  
int* array;
double elapsed = 0;

//Function Prototypes
int partition(int* array, int low, int high);
void quickSort(int* array, int low, int high);
void swap(int* array, int x, int y);
void printArray(int* array, int size);
void Usage(const char* prog_name); 

/**
 * Preps the call to quicksort by checking the arguments for serial 
 * or parallel execution. Then randomly generates an array of the provided
 * size (if not provided, size 8 is default)
 * 
 * Parallel quicksort is not implemented, please use serial sort
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
            break;

        case 2:

            if (strcmp(argv[1], "-s") == 0){
                parallel = false;
                arraySize = 8;
            }//if

            else{
                //get size of array from command line
                arraySize = strtol(argv[1], NULL, 10);
            }//else

            break;

        case 3:
            
            if (strcmp(argv[1], "-s") == 0){
                parallel = false;
                arraySize = 8;
            }//if

            //get size of array from command line
            arraySize = strtol(argv[2], NULL, 10);

            break;      

        default:
            Usage(argv[0]);
            return EXIT_SUCCESS;   

    }//switch

    array = malloc(sizeof(int) * arraySize);
    
    srand((unsigned) time(NULL));

    //fill the array with random ints between 0 and MAX
    for (int i = 0 ; i < arraySize ; i++){
        array[i] =  (double)rand() / (double)(RAND_MAX / MAX);
    }//for

    if (parallel){
        printf("Parallel quicksort not implemented, please use serial (\"-s\")\n");
        free(array);
        return EXIT_SUCCESS;

        //leave this here for when parallel implemented
        printf("\nParallel time on array of size %d:\n"
            "%f seconds\n", arraySize, elapsed);

    }//if

    else{
       
        clock_gettime(CLOCK_MONOTONIC, &start);
        quickSort(array, 0, arraySize-1); 
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
 * Takes the last element of an array as the pivot, places 
 * the pivot element at its correct position in sorted 
 * array, and places all smaller (smaller than pivot) 
 * to left of pivot and all greater elements to right 
 * of pivot 
 * 
 * From https://www.geeksforgeeks.org/quick-sort/
 * 
 * @param array: the array to be partitioned
 * @param low: starting index into array for partition
 * @param high: ending index into array for partition
 */
int partition (int* array, int low, int high){ 
    
    int pivot = array[high];    // pivot
    int i = (low - 1);  // Index of smaller element 
  
    for (int j = low; j <= high - 1; j++){ 
        // If current element is smaller than the pivot 
        if (array[j] < pivot){ 
            i++;    // increment index of smaller element 
            swap(array, i, j); 
        }//if

    }//for 

    swap(array, i + 1, high); 
    return (i + 1); 

}//partition 
  
/**
 * The main function that implements QuickSort through 
 * recursive calls and use of partition()
 * 
 * From https://www.geeksforgeeks.org/quick-sort/
 * 
 * @param array: Array to be sorted
 * @param low: Starting index
 * @param high: Ending index 
 * @return void
 */
void quickSort(int* array, int low, int high){ 
    
    if (low < high){ 
        
        //pi is partitioning index, array[p] is now at right place
        int pi = partition(array, low, high); 
  
        //Separately sort elements before and after the partition
        quickSort(array, low, pi - 1); 
        quickSort(array, pi + 1, high); 

    }//if

}//quicksort 

/**
 * Swaps the two elements located at the provided indices of the array.
 * The array to be sorted is assumed to be of double precision numbers
 * 
 * @param array: pointer to the array of ints where the swap is to occur
 * @param x: first indice of array that needs to be swapped
 * @param y: second indice of array that needs to be swapped
 * @return void
 */ 
void swap(int* array, int x, int y) {
   
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
void Usage(const char* prog_name){
   fprintf(stderr, "usage:   %s <-s> <n>\n", prog_name);
   fprintf(stderr, "  's':  run serial quicksort sort\n");
   fprintf(stderr, "   n:   number of elements in list\n");
}//Usage
