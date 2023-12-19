#include <stdio.h>
#include <stdlib.h>
#include "partition.h"


PivotedArrays partition(int* A, int size, int pivot) {
    PivotedArrays result;
    result.left = (int*)malloc(size * sizeof(int));
    result.middle = (int*)malloc(size * sizeof(int));
    result.right = (int*)malloc(size * sizeof(int));

    int leftIndex = 0, middleIndex = 0, rightIndex = 0;

    for(int i = 0 ;i < size ; i++){
        //Elements less than pivot
        if(A[i] < pivot){
            result.left[leftIndex++] = A[i];
        }
        //Elements equal to pivot
        else if(A[i] == pivot){
            result.middle[middleIndex++] = A[i];
        }
        //Elements greater than pivot
        else{
            result.right[rightIndex++] = A[i];
        }
    }

    result.leftSize = leftIndex;
    result.middleSize = middleIndex;
    result.rightSize = rightIndex;
    return result;
}



int kselect(int* A, int size, int k , int pivot) {
    if (k > 0 && k <= size) {
    
        // Partition the array using the pivot
        PivotedArrays result = partition(A, size, pivot);

        // Recursively find k-th smallest in the appropriate partition
        if (k <= result.leftSize) {
            return kselect(result.left, result.leftSize, k , pivot) ;
        } 
        
        else if (k <= result.leftSize + result.middleSize) {
            return pivot;
        } 
        
        else {
            return kselect(result.right, result.rightSize, k - result.leftSize - result.middleSize , pivot);
        }
    free(result.left);
    free(result.middle);
    free(result.right);

    } 
    
    else {
        printf("Invalid value of k.\n");
        exit(EXIT_FAILURE);
    }
}


// int main() {
//     int A[] = {5, 12, 3, 8, 1, 7, 9, 4 , 2 , 2, 0, 0, 86, -1};
//     int size = sizeof(A) / sizeof(A[0]);
//     int k = 3;
//     printf("Original Array: ");
//     for (int i = 0; i < size; ++i) {
//         printf("%d ", A[i]);
//     }


//     int kSmallest = kselect(A, size, k);
//     printf("\nThe %d-th smallest element is: %d\n", k, kSmallest);

//     return 0;
// }

// int main() {
//     // Seed for random numbers
//     srand(time(NULL));

//     // Generate a random array
//     int size = 10;
//     int* array = (int*)malloc(size * sizeof(int));
//     for (int i = 0; i < size; ++i) {
//         array[i] = rand() % 1000;  // Random numbers between 0 and 999
//     }

//     // Print the generated array
//     printf("Original array: ");
//     for (int i = 0; i < size; ++i) {
//         printf("%d ", array[i]);
//     }
//     printf("\n");

//     // Choose k
//     int k = 5;

//     // Find the k-th smallest element
//     int result = kselect(array, size, k);

//     // Print the result
//     printf("The %d-th smallest element is: %d\n", k, result);

//     // Free allocated memory
//     free(array);

//     return 0;
// }