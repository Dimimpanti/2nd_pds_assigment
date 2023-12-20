#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../partition.h"
#include "../structs.h"
#include "masterProcess.h"
#include "slaveProcess.h"


/**
 * @brief Every process reads it's part of the array from the file
 * 
 * @param fileName   The name of the file
 * @param rank       The rank of the current process
 * @param worldSize  The total number of processes
 * @param numValues  The number of values that the current process will handle
 * @return int*      
 */
int *readFromFile(char *fileName, int rank, int worldSize, int *numValues) {
   
    FILE *file = fopen(fileName, "r");  // Open the file for reading

    // Check if the file is open
    if (file == NULL) {

        if (rank == 0) {
            printf("\nCan not find file %s. Program will now exit...\n\n", fileName);
        }

        //Exit the program
        MPI_Finalize();
        exit(0);
    }

    int numOfInts = 0;  // The number of elements in the file
    int tmp;        // Temporary variable for reading the file

    // count the number of integers in the file
    while (fscanf(file, "%d", &tmp) != EOF) {
        numOfInts++;
    }
    
    // Move the file pointer to the start of the file
    fseek(file, 0, SEEK_SET);

    // Calculate the size of the array that each process will have to handle. The remainder is discarded.
    *numValues = numOfInts / worldSize;
    
    printf("Process %d will handle %d numbers\n", rank, *numValues);

    // skip the integers that the previous processes will handle
    for (int i = 0; i < rank * (*numValues); i++) {
        fscanf(file, "%d", &tmp);
    }
    

    // Allocate memory for the local array
    int *values = (int *)malloc(*numValues * sizeof(int));  // TODO free this memory

    // Read the integers for the current process
    for (int i = 0; i < *numValues; i++) {
        fscanf(file, "%d", &values[i]);
    }

    // Close the file
    fclose(file);

    return values;
}


int main(int argc, char **argv) {

    //Initialize the MPI communication
    MPI_Init(&argc, &argv);

    double start, end;  // Time measuring
    
    Info info;  // The info struct holds information for every process 
    int min_rank = 0;
    int max_rank = info.world_size;
    
    
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &info.world_size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &info.world_rank);



    // Read data from the file
    info.A = readFromFile(argv[1], info.world_rank, info.world_size, &info.size);

     // Time measurement starts after the points are read
    if (info.world_rank == 0){
        start = MPI_Wtime();
    }

    if (info.world_rank == 0) {
        masterProcess(info.world_rank, min_rank, max_rank, &info, MPI_COMM_WORLD);
    } else {
        slaveProcess(info.world_rank, min_rank, max_rank, &info, MPI_COMM_WORLD);
    }


    // Wait here for all the processes to finish before time measurement
    MPI_Barrier(MPI_COMM_WORLD);
   
   //Time measurement ends here
    if (info.world_rank == 0){
        end = MPI_Wtime();

        printf("\nTime for execution: %.6f\n", end - start);

    }

    // Finalize the MPI environment.
    MPI_Finalize();

    return 0;
}
