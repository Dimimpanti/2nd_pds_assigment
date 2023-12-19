#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <mpi.h>
#include "partition.h"
#include "slaveProcess.h"

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
    
    PivotedArrays result;

    //split every process to left and right subarrays accordingly to pivot 
    result = partition(info->A, info->size, info->pivot);

    int partitionResult[3] = {result.leftSize, result.middleSize, result.rightSize};
    // Send the sizes of the partitioned array to master process
    MPI_Send(
        partitionResult,
        3,
        MPI_INT,
        master_rank,
        10,
        communicator
    );

    // A variable that if it is one we keep the left aarray ,if it is 2 we keep the middle
    // and if it is 3 we keep the right
    int status;

    MPI_Bcast(
        &status,
        1,
        MPI_INT,
        master_rank,
        communicator
    );

    if(status == 1){
        free(info->A);
        free(result.right);
        free(result.middle);

        info->A = result.left;
        info->size = result.leftSize;

         MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
        //slaves stop here
        if(status == 4){
            return;
        }
        MPI_Gather(info->A, info->size, MPI_INT, gatheredData, info->size, MPI_INT, master_rank, communicator);



    } else if(status == 2){
        free(partitionResult);  
        free(result.left);
        free(result.right);
        free(result.middle);

        return;

    } else if(status == 3){
        free(info->A);
        free(result.left);
        free(result.middle);

        info->A = result.right;
        info->size = result.rightSize;

         MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
        //slaves stop here
        if(status == 4){
            return;
        }
    }

     


    slaveProcess(master_rank, min_rank, max_rank, info, communicator);
    
    free(partitionResult);
}
