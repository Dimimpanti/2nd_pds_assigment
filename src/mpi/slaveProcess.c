#include "slaveProcess.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "partition.h"


/**
 * @brief Balance the number of ints that remain in the array of each process
 * 
 * @param ints         The array of ints
 * @param numInts      The number of ints of the process
 * @param worldSize    The total number of processes
 * @param min_rank     The smallest rank
 * @param max_rank     The biggest rank
 * @param rank         The rank of the current process
 * @param communicator The MPI communicator
 * 
 */
void balancePointsSlave(int **ints, int *numInts, int worldSize, int min_rank, int max_rank, int rank, MPI_Comm communicator){
    
    // print the initial ints of each process
    // printf("\t\t\t\tinitial ints\n\t\t\t\t");

    // for(int i = 0; i < *numInts; i++){
    //     printf("%d ", ints[i]);
    // }

    // printf("\n");

    // Vectors that will hold the receive and send counts for each process
    int *sendCounts = (int *)malloc(worldSize * sizeof(int));  // TODO: free this
    int *recvCounts = (int *)malloc(worldSize * sizeof(int));  // TODO: free this
    
    MPI_Request request[2];

    MPI_Irecv(
        sendCounts,
        worldSize,
        MPI_INT,
        min_rank,
        0,
        communicator,
        request
    );
    
    MPI_Irecv(
        recvCounts,
        worldSize,
        MPI_INT,
        min_rank,
        1,
        communicator,
        request + 1
    );

    // Wait for the receives to finish
    MPI_Waitall(2, request, MPI_STATUSES_IGNORE);

    MPI_Request *exchangeRequest = (MPI_Request *)malloc(worldSize * sizeof(MPI_Request));  // TODO: free this

    // based on the send and receive arrays start the exchange of ints between the processes
    for (int i = 0; i < worldSize; i++){
        if(sendCounts[i] != 0){
            MPI_Isend(
                *ints + *numInts - sendCounts[i],
                sendCounts[i],
                MPI_INT,
                i,
                0,
                communicator,
                exchangeRequest + i
            );

            *numInts -= sendCounts[i];

        } else if(recvCounts[i] != 0){
            int *receivedInts = (int *)malloc(recvCounts[i] * sizeof(int));  // TODO free this

            MPI_Recv(
                receivedInts,
                recvCounts[i],
                MPI_INT,
                i,
                0,
                communicator,
                MPI_STATUS_IGNORE
            );

            *numInts += recvCounts[i];

            *ints = (int *)realloc(*ints, *numInts * sizeof(int));  // TODO free this

            memcpy(*ints + *numInts - recvCounts[i], receivedInts, recvCounts[i] * sizeof(int));

            free(receivedInts);
        }
    }

    // Wait for the sends to finish
    for (int i = 0; i < worldSize; i++){
        if(sendCounts[i] != 0){
            MPI_Wait(exchangeRequest + i, MPI_STATUS_IGNORE);
        }
    }

    free(sendCounts);
    free(recvCounts);
    free(exchangeRequest);

    // printf("\t\t\t\tfinal ints\n\t\t\t\t");

    // for(int i = 0; i < *numInts; i++){
    //     printf("%d ", ints[i]);
    // }
    // printf("\n");
}


/**
 * This is the function that all processes, except the master, run. In this function all the computations and
 * communications with other processes are handled.
 *
 * @param master_rank The rank of the master thread for the current communicator
 * @param min_rank The minimum rank of this communicator
 * @param max_rank The maximum rank of this communicator
 * @param info The info struct of the current process
 * @param communicator The MPI communicator.
 */
void slaveProcess(int master_rank, int min_rank, int max_rank, Info *info, MPI_Comm communicator){

    // Broadcast the pivot to all the other processes
    MPI_Bcast(
            &info->pivot,
            1, 
            MPI_INT,
            0,
            MPI_COMM_WORLD
    ); 
    
    PivotedArrays *result;

    //split every process to left, middle and right sub-arrays accordingly to pivot 
    result = partition(info->A, info->size, info->pivot);  // TODO: free this

    int partitionResult[3] = {result->leftSize, result->middleSize, result->rightSize};
    
    // Send the sizes of the partitioned array to master process
    MPI_Send(
        partitionResult,
        3,
        MPI_INT,
        master_rank,
        10,
        communicator
    );

    // A variable that if it is one we keep the left array, if it is 2 we keep the middle
    // and if it is 3 we keep the right
    int status;

    // Receive the status from the master process
    MPI_Bcast(
        &status,
        1,
        MPI_INT,
        master_rank,
        communicator
    );

    // If the status is 1 we keep the left array
    if(status == 1){
        free(info->A);
        free(result->right);
        free(result->middle);

        info->A = result->left;
        info->size = result->leftSize;

        // Receive the status from the master process again
        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );

        // printf("\t\tProcess %d has %d ints\n\t\t", info->world_rank, info->size);


        // If the status is 4 master gathers the data and all the other processes stop
        if(status == 4){

            // Gather the data from all the processes to the master process
            MPI_Gatherv(
                info->A,
                info->size,
                MPI_INT,
                NULL,
                NULL,
                NULL,
                MPI_INT,
                master_rank,
                communicator
            );

            // Free the memory
            free(info->A);
            free(result);

            // and exit
            return;
        }
        
    } else if(status == 2){
        // If the status is the k-th smallest is the pivot

        // free the memory
        free(info->A);
        free(result->left);
        free(result->right);
        free(result->middle);
        free(result);

        // and exit
        return;

    } else if(status == 3){
        free(info->A);
        free(result->left);
        free(result->middle);

        info->A = result->right;
        info->size = result->rightSize;
        
        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
        
        // If the status is 4 master gathers the data and all the other processes stop
        if(status == 4){
            
            // Gather the data from all the processes to the master process
            MPI_Gatherv(
                info->A,
                info->size,
                MPI_INT,
                NULL,
                NULL,
                NULL,
                MPI_INT,
                master_rank,
                communicator
            );

            // Free the memory
            free(info->A);
            free(result);

            // and exit
            return;
        }
    }


    // Balance the number of ints that remain in the array of each process
    balancePointsSlave(&info->A, &info->size, info->world_size, min_rank, max_rank, info->world_rank, communicator);

    slaveProcess(master_rank, min_rank, max_rank, info, communicator);
    
    free(result);
}