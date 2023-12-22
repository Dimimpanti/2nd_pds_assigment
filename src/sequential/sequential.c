#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../partition.h"

int *readFromFile(FILE *file, int *numOfInts) {
    
    int tmp;            // Temporary variable for reading the file

    // count the number of integers in the file
    while (fscanf(file, "%d", &tmp) != EOF) {
        (*numOfInts)++;
    }
    
    // Move the file pointer to the start of the file
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the local array
    int *values = (int *)malloc(*numOfInts * sizeof(int));  // TODO free this memory

    // Read the integers for the current process
    for (int i = 0; i < *numOfInts; i++) {
        fscanf(file, "%d", &values[i]);
    }

    return values;
}

int main(int argc, char **argv) {
    
    if (argc != 3) {
        printf("Usage: %s <input_file> <k>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        printf("Can not open file \"%s\".\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    int k = atoi(argv[2]);  // parse the k-th smallest element from the command line arguments

    int numOfInts = 0;  // The number of elements in the file

    struct timeval t1, t2;
    double elapsedTime;
    
    gettimeofday(&t1, NULL);

    // read the points from the file and store them in the array
    int *A = readFromFile(file, &numOfInts);

    int result = kselect(A, numOfInts, k);

    // measure the time
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

    printf("The %d-th smallest element is %d\n", k, result);

    printf("\nTime for sequential execution:: %.6fs.\n", elapsedTime / 1000.0);

    free(A);
    return 0;
}