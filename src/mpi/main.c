#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../partition.h"



int *readFromFile(char *fileName, int rank, int worldSize, int *values) {
   
   FILE *file = NULL;  // File pointer
    file = fopen("data.txt", "r");  // Open the file for reading

    // Check if the file is open
    if (file == NULL) {
        if (rank == 0) {
            printf("\nCan not find file %s. Program will now exit...\n\n", fileName);
        }

        //Exit the program
        MPI_Finalize();
        exit(0);
    }

    int numOfInts;  // The number of elements in the file

   // Read the total elements from the file
    fread(&numOfInts, sizeof(int), 1, file);

    // Calculate the size of the array that each process will have to handle 
    int intsPerProcess = numOfInts / worldSize;
    
    printf("Each process will handle %d integers\n", intsPerProcess);

    // Move file pointer to the start position for the current process
    fseek(file, rank * intsPerProcess * sizeof(int), SEEK_SET);

    // Allocate memory for the local array
    values = (int *)malloc(intsPerProcess * sizeof(int));

    // Read the integers for the current process
    fread(values, sizeof(int), intsPerProcess, file);

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
    info.A = readFromFile(argv[1], info.world_rank, info.world_size, info.A);

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
