#include "masterProcess.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "partition.h"


#define endRecursion 100

/**
 * This is the function of the master processes. Here the k-th smallest value is calculated and  all the processes
 * are controlled.
 *
 * @param master_rank The rank of the master process
 * @param min_rank The smallest rank
 * @param max_rank The biggest rank
 * @param info The info struct of the process
 * @param communicator The communicator of the processes grouped together
 */
void masterProcess(int master_rank, int min_rank, int max_rank, Info *info, MPI_Comm communicator , int *kthSmallest){

    info->pivot = info->A[0] ;
    
    // Broadcast the pivot to all the other processes
    MPI_Bcast(
            &info->pivot,
            1, 
            MPI_INT,
            0,
            MPI_COMM_WORLD
    ); 

    // Allocate space for all the requests objects
    MPI_Request *requests;
    requests = (MPI_Request *) malloc((max_rank - min_rank) * sizeof (MPI_Request));  // MEMORY

    // Receive the elements at the left ,right and pivotted arrays from the other processes.

    int* partitionResult = (int*)malloc((max_rank - min_rank + 1) * 3 * sizeof(int));

    for (int i = 1; i <= max_rank - min_rank; ++i) {
        MPI_Irecv(
            partitionResult + 3 * i,
            3, 
            MPI_INT,
            i ,
            10,
            communicator,
            requests + i - 1
        );
    }

    
    //Declare and initialize the result array
    // PivotedArrays* result = (PivotedArrays*)malloc((max_rank - min_rank) * sizeof(PivotedArrays));  
    PivotedArrays result;

    //split every process to left and right subarrays accordingly to pivot 
    result = partition(info->A, info->size, info->pivot);
    
    partitionResult[0] = result.leftSize;
    partitionResult[1] = result.middleSize;
    partitionResult[2] = result.rightSize;

    // Wait for all the k-th smallest values of each process to be received
    for (int i = 0; i < max_rank - min_rank; ++i) {
        MPI_Wait(requests + i, MPI_STATUS_IGNORE);
    }

    int left=0, middle=0, right=0;
    
    for(int i = 0 ; i < 3 * (max_rank - min_rank + 1) ; i+=3){
        left += partitionResult[i];
        middle += partitionResult[i+1];
        right += partitionResult[i+2];
    }

    // A variable that if it is one we keep the left aarray ,if it is 2 we keep the middle
    // and if it is 3 we keep the right
    int status;
    
    if(info->k <= left){
        free(info->A);
        free(result.right);
        free(result.middle);

        info->A = result.left;
        info->size = result.leftSize;


       status = 1;

        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
    
        //ending condition
        if(left<=endRecursion){
       
            status = 4;
            //Broadcast to all the slaves if they should stop or not
            MPI_Bcast(
                &status,
                1,
                MPI_INT,
                master_rank,
                communicator
            );
        
        int *gatheredData = (int *) malloc(left * sizeof (int));  // MEMORY
       
        MPI_Gather(info->A, info->size, MPI_INT, gatheredData, info->size, MPI_INT, master_rank, communicator);

        //find the k-th smallest value using the kselect function for all the processes
        *kthSmallest = kselect(gatheredData, left, info->k , info->pivot);
   
        

        free(partitionResult);
        free(requests);
        free(result.left);
        free(result.right);
        free(result.middle);
        free(gatheredData);

        
        return;
        }
        
        
    }
    else if(info->k <= left + middle){
        *kthSmallest = info->pivot;
        status = 2;

        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
        
        free(partitionResult);
        free(requests);
        free(result.left);
        free(result.right);
        free(result.middle);
        
        return;
    }


    else{
        free(info->A);
        free(result.left);
        free(result.middle);

        info->A = result.right;
        info->size = result.rightSize;
        info->k -= left + middle;

        status = 3;

        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
    
        //ending condition
        if(right<=endRecursion){
       
            //Broadcast to all the slaves if they should stop or not
            status = 4;

            MPI_Bcast(
                &status,
                1,
                MPI_INT,
                master_rank,
                communicator
            );

              
        int *gatheredData = (int *) malloc(right * sizeof (int));  // MEMORY
       
        MPI_Gather(info->A, info->size, MPI_INT, gatheredData, info->size, MPI_INT, master_rank, communicator);

        //find the k-th smallest value using the kselect function for all the processes
        *kthSmallest = kselect(gatheredData, right, info->k , info->pivot);
    

            free(partitionResult);
            free(requests);
            free(result.left);
            free(result.right);
            free(result.middle);
            free(gatheredData);

        return;
    }
    
}
    
    
    masterProcess(master_rank, min_rank, max_rank, info, communicator , kthSmallest);

    free(partitionResult);
    free(requests);
}

