#ifndef STRUCTS_H   /* Include guard */
#define STRUCTS_H 

#include <mpi.h>


typedef struct{
    int* left;
    int leftSize;
    int* middle;
    int middleSize;
    int* right;
    int rightSize;
} PivotedArrays;


typedef struct{
    int world_size;  // The size of the world (number of processes)
    int world_rank;  // The rank of the current process
    
    int* A; // The array of each process 
    int size; // The size of the array each process has
    int k; // The k-th smallest value of the array
    int pivot; //Pivot element selected by the master process
}Info;


#endif