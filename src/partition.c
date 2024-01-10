#include <stdio.h>
#include <stdlib.h>
#include "partition.h"


PivotedArrays *partition(int* A, int size, int pivot) {
    PivotedArrays *result = (PivotedArrays*)malloc(sizeof(PivotedArrays));

    result->left = (int*)malloc(size * sizeof(int));
    result->middle = (int*)malloc(size * sizeof(int));
    result->right = (int*)malloc(size * sizeof(int));

    int leftIndex = 0, middleIndex = 0, rightIndex = 0;

    for(int i = 0 ;i < size ; i++){
        //Elements less than pivot
        if(A[i] < pivot){
            result->left[leftIndex++] = A[i];
        }
        //Elements equal to pivot
        else if(A[i] == pivot){
            result->middle[middleIndex++] = A[i];
        }
        //Elements greater than pivot
        else{
            result->right[rightIndex++] = A[i];
        }
    }

    result->leftSize = leftIndex;
    result->middleSize = middleIndex;
    result->rightSize = rightIndex;

    return result;
}


/**
 * @brief  Find the k-th smallest element in an array
 * 
 * @param A     The array
 * @param size  The size of the array
 * @param k     The k-th smallest element
 * @return int 
 */
int kselect(int* A, int size, int k) {

    // Pivot is the first element of the array
    int pivot = A[0];

    if (k > 0 && k <= size) {
    
        // Partition the array using the pivot
        PivotedArrays *result = partition(A, size, pivot);

        // Recursively find k-th smallest in the appropriate partition
        if (k <= result->leftSize) {
            int res = kselect(result->left, result->leftSize, k);

            free(result->left);
            free(result->middle);
            free(result->right);
            free(result);

            return res;
        
        } else if (k <= result->leftSize + result->middleSize) {
            free(result->left);
            free(result->middle);
            free(result->right);
            free(result);

            return pivot;
        
        } else {
            int res = kselect(result->right, result->rightSize, k - result->leftSize - result->middleSize);

            free(result->left);
            free(result->middle);
            free(result->right);
            free(result);

            return res;
        }

    } 
    
    else {
        printf("Invalid value of k: %d.\n", k);

        printf("Invalid value of k: %d.\n", k);

        exit(EXIT_FAILURE);
    }
}
