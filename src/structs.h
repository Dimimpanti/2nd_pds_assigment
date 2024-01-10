#ifndef STRUCTS_H   /* Include guard */
#define STRUCTS_H 


/**
 * @brief Struct that holds the partition results
 * 
 * @param left       The left partitioned array (< pivot)
 * @param leftSize   The size of the left partitioned array
 * @param middle     The middle partitioned array (== pivot)
 * @param middleSize The size of the middle partitioned array
 * @param right      The right partitioned array  (> pivot)
 * @param rightSize  The size of the right partitioned array
 * 
 */
typedef struct{
    int* left;
    int leftSize;
    int* middle;
    int middleSize;
    int* right;
    int rightSize;
} PivotedArrays;


/**
 * @brief The struct that holds the information that each process needs
 * 
 * @param world_size   The size of the world (number of processes)
 * @param world_rank   The rank of the current process
 * @param A            The array of each process
 * @param size         The size of the array each process has
 * @param k            The k-th smallest value of the array
 * @param pivot        Pivot element selected by the master process
 * 
 */
typedef struct{
    int world_size;  
    int world_rank;
    
    int *A;
    int size;
    int k;
    int pivot;
}Info;


#endif