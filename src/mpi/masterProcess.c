#include "masterProcess.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "partition.h"


#define END_RECURSION 100000

/**
 * @brief Balance the number of points that remain in the array of each process
 * 
 * @param points        The array of points
 * @param masterPoints  The number of points that the master process has
 * @param numPoints     The total number of points
 * @param worldSize     The total number of processes
 * @param min_rank      The smallest rank
 * @param max_rank      The biggest rank
 * @param pointsPerProc The array that holds the number of points of each process
 * @param communicator  The communicator of the processes grouped together
 * 
 */
void balancePointsMaster(int **points, int *masterPoints, int numPoints, int worldSize, int min_rank, int max_rank, int *pointsPerProc, MPI_Comm communicator){
    
    // the master process must calculate the exchange of points between the processes so that the number of points
    // in each process is balanced

    // the number of points that each process should have
    int pointsPerProcess = numPoints / worldSize;

    // the remainder of the division
    int remainder = numPoints % worldSize;

    // the number of points that each process should have after the exchange
    int *newPointsPerProc = (int *)malloc(worldSize * sizeof (int));  // TODO free this DONE

    // fill the newPointsPerProc array with the pointsPerProcess value
    for (int i = 0; i < worldSize; ++i) {
        newPointsPerProc[i] = pointsPerProcess;
    }

    // add the remainder shared equally to all the processes
    for (int i = 0; i < remainder; ++i) {
        newPointsPerProc[i]++;
    }

    // the number of points that each process will give to the other processes
    int *pointsToGive = (int *)malloc(worldSize * sizeof (int));  // TODO free this DONE

    // the number of points that each process will receive from the other processes 
    int *pointsToReceive = (int *)malloc(worldSize * sizeof (int));  // TODO free this DONE

    // calculate the pointsToGive and pointsToReceive arrays
    for (int i = 0; i < worldSize; ++i) {
        if (pointsPerProc[i] > newPointsPerProc[i]) {
            pointsToGive[i] = pointsPerProc[i] - newPointsPerProc[i];
            pointsToReceive[i] = 0;
        } else {
            pointsToGive[i] = 0;
            pointsToReceive[i] = newPointsPerProc[i] - pointsPerProc[i];
        }
    }

    free(newPointsPerProc);


    // This 2D array holds the points that each process will give to the other processes
    int **pointsToSend = (int **)malloc(worldSize * sizeof (int *));  // TODO free this DONE

    // Allocate memory for the pointsToSend array
    for (int i = 0; i < worldSize; ++i) {
        pointsToSend[i] = (int *)calloc(worldSize, sizeof (int));  // TODO free this DONE
    }
    
    int pointsToReceiveIndex = 0;

    // fill the pointsToSend array
    for (int i = 0; i < worldSize; i++) {
        int give = pointsToGive[i];

        if (give == 0) {
            continue;
        }

        // find the first process that has points to receive
        for (int j = pointsToReceiveIndex; j < worldSize; j++) {
            if (i != j) {

                // if the process has points to receive
                if (pointsToReceive[j] != 0) {

                    // if the process has less points to receive than the process has to give
                    if (give > pointsToReceive[j]) {

                        // satisfy the process with the points it needs
                        pointsToSend[i][j] = pointsToReceive[j];

                        // decrease the number of points that the process has to give
                        give -= pointsToReceive[j];

                        // the process has no more points to receive
                        pointsToReceive[j] = 0;
                        
                        // increase the index of the process that has points to receive
                        pointsToReceiveIndex++;

                    }
                    // if the process has equal points to receive 
                    else if (give == pointsToReceive[j]) {

                        // satisfy the process with the points it needs
                        pointsToSend[i][j] = give;

                        // the process has no more points to receive
                        pointsToReceive[j] = 0;

                        // increase the index of the process that has points to receive
                        pointsToReceiveIndex++;

                        break;

                    } 
                    // if the process has more points to receive than the process has to give
                    else {

                        // satisfy the process with the points it needs
                        pointsToSend[i][j] = give;

                        // decrease the number of points that the process has to receive
                        pointsToReceive[j] -= give;

                        break;
                    }
                } else {  // if the process has no points to receive

                    // increase the index of the process that has points to receive
                    pointsToReceiveIndex++;
                }

            } else {  // if the process is the same as the process that has points to give

                // increase the index of the process that has points to receive
                pointsToReceiveIndex++;
            }
        }
        
        // if all the processes have received the points they need we can stop the loop
        if (pointsToReceiveIndex == worldSize) {
            break;
        }
    }

    free(pointsToGive);
    free(pointsToReceive);

    // Each row of the pointsToSend array holds the number of points that each process will give to the other processes
    // Each column of the pointsToSend array holds the number of points that each process will receive from the other processes
    // Send the row and the column of the pointsToSend array to the corresponding processes

    // Allocate memory for the requests array
    MPI_Request *requests = (MPI_Request *)malloc((worldSize - 1) * 2 * sizeof (MPI_Request));  // TODO free this DONE

    // Send the row of the pointsToSend array to the corresponding processes
    for (int i = 1; i < worldSize; ++i) {
        MPI_Isend(
            pointsToSend[i],
            worldSize,
            MPI_INT,
            i,
            0,
            communicator,
            requests + i - 1
        );
    }

    // Send the column of the pointsToSend array to the corresponding processes
    int **columns = (int **)malloc(worldSize * sizeof (int *));  // TODO free this DONE

    for (int i = 0; i < worldSize; ++i) {
        columns[i] = (int *)malloc(worldSize * sizeof (int));  // TODO free this DONE
    }

    // transpose the pointsToSend array to get the columns
    for (int i = 0; i < worldSize; ++i) {
        for (int j = 0; j < worldSize; ++j) {
            columns[i][j] = pointsToSend[j][i];
        }
    }

    // send the columns to the corresponding processes
    for (int i = 1; i < worldSize; ++i) {

        MPI_Isend(
            columns[i],
            worldSize,
            MPI_INT,
            i,
            1,
            communicator,
            requests + i + worldSize - 2
        );
    }

    // wait for all the sends to complete
    MPI_Waitall(2 * (worldSize - 1), requests, MPI_STATUS_IGNORE);
    free(requests);

    requests = (MPI_Request *)malloc(worldSize * sizeof (MPI_Request));  // TODO free this DONE

    // based on the send and receive arrays start the exchange of points between the processes
    for (int j = 0; j < worldSize; j++) {
        if (pointsToSend[0][j] != 0) {
            MPI_Isend(
                *points + *masterPoints - pointsToSend[0][j],
                pointsToSend[0][j],
                MPI_INT,
                j,
                0,
                communicator,
                requests + j
            );

            *masterPoints -= pointsToSend[0][j];
        
        } else if (columns[0][j] != 0) {
            int *receivedPoints = (int *)malloc(columns[0][j] * sizeof(int));  // TODO free this DONE

            MPI_Recv(
                receivedPoints,
                columns[0][j],
                MPI_INT,
                j,
                0,
                communicator,
                MPI_STATUS_IGNORE
            );

           *masterPoints += columns[0][j];

           int numPoints = *masterPoints;

            int *tmp = (int *)realloc(*points, numPoints * sizeof(int));

            if (tmp == NULL) {
                printf("Error reallocating memory.\n");
                exit(EXIT_FAILURE);
            } else {
                *points = tmp;
            }

            memcpy(*points + numPoints - columns[0][j], receivedPoints, columns[0][j] * sizeof(int));

            free(receivedPoints);
        }
    }


    // wait for all the sends to complete
    for (int i = 0; i < worldSize; ++i) {
        if (pointsToSend[0][i] != 0) {
            MPI_Wait(requests + i, MPI_STATUS_IGNORE);
        }
    }

    free(requests);

    // free the pointsToSend array
    for (int i = 0; i < worldSize; ++i) {
        free(pointsToSend[i]);
    }

    free(pointsToSend);

    // free the columns array
    for (int i = 0; i < worldSize; ++i) {
        free(columns[i]);
    }

    free(columns);
}

/**
 * This is the function of the master processes. Here the k-th smallest value is calculated and
 * all the processes are controlled.
 *
 * @param master_rank   The rank of the master process
 * @param min_rank      The smallest rank
 * @param max_rank      The biggest rank
 * @param info          The info struct of the process
 * @param communicator  The communicator of the processes grouped together
 * @param kthSmallest   The k-th smallest value of the array (the result)
 */
void masterProcess(int master_rank, int min_rank, int max_rank, Info *info, MPI_Comm communicator , int *kthSmallest){


    // Master process chooses the pivot ...
    info->pivot = info->A[0];
    
    // ... and broadcasts the pivot to all the other processes
    MPI_Bcast(
        &info->pivot,
        1, 
        MPI_INT,
        0,
        MPI_COMM_WORLD
    ); 


    /* 
     * Each process will partition its array to left ,middle and right sub-arrays accordingly to pivot
     * at the end of the partitioning each process sends the sizes of the sub-arrays to the master
     * process. The receive is non-blocking so the master process can continue its computations and 
     * potentially gain some time.
     */
    
    // Allocate space for all the requests objects
    MPI_Request *requests = (MPI_Request *) malloc((info->world_size - 1) * sizeof (MPI_Request));  // TODO free this

    // Allocate space for the partition results
    int* partitionResult = (int*)malloc(info->world_size * 3 * sizeof(int));  // TODO free this

    // Receive the number of elements of the left, right and middle arrays of the other processes.
    for (int i = 1; i <= info->world_size - 1; ++i) {
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

    
    // result holds the partitioned arrays for the master process
    PivotedArrays *result;


    //split every process to left and right sub-arrays accordingly to pivot 
    result = partition(info->A, info->size, info->pivot);  // TODO free this
    
    // Store the sizes of the left, right and middle arrays of the master process
    partitionResult[0] = result->leftSize;
    partitionResult[1] = result->middleSize;
    partitionResult[2] = result->rightSize;

    // Wait for all the receives to complete
    for (int i = 0; i < info->world_size - 1; ++i) {
        MPI_Wait(requests + i, MPI_STATUS_IGNORE);
    }


    // Left , middle and right hold the sizes of the left , middle and right sub-arrays of all the processes
    int left=0, middle=0, right=0;
    

    // Calculate the total sizes of the left, right and middle arrays
    for(int i = 0 ; i < 3 * info->world_size ; i += 3){
        left += partitionResult[i];
        middle += partitionResult[i+1];
        right += partitionResult[i+2];
    }
    

    // A variable that if it is one we keep the left array ,if it is 2 we keep the middle
    // and if it is 3 we keep the right
    int status;

    int totalPoints;
    int *pointsPerProc = (int *) malloc(info->world_size * sizeof (int));  // TODO free this
    
    // At this point the master process has all the information it needs to decide what to do next
    if(info->k <= left){
        // if the k-th smallest value is in the left sub-array we keep the left sub-array

        free(info->A);
        free(result->right);
        free(result->middle);

        // master process keeps the left sub-array
        info->A = result->left;
        info->size = result->leftSize;

        totalPoints = left;

        // fill in pointsPerProc
        for (int i = 0; i < info->world_size; ++i) {
            pointsPerProc[i] = partitionResult[3 * i];
        }

        status = 1;  // 1 means keep the left sub-array

        // Broadcast to all the slaves to keep the left sub-array
        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );

    
        // If the left sub-array is small enough we can stop the recursion and find the k-th smallest value by sorting the array
        if(left <= END_RECURSION){
       
            status = 4;

            // Broadcast to all the slaves if they should stop or not
            MPI_Bcast(
                &status,
                1,
                MPI_INT,
                master_rank,
                communicator
            );


            // This array will hold all the elements of the left sub-arrays of all the processes
            int *gatheredData = (int *) malloc(left * sizeof (int));  // TODO free this

            // the number of elements to receive from each process
            int *recvCounts = (int *) malloc(info->world_size * sizeof (int));  // TODO free this

            // the displacements of the elements of each process
            int *displs = (int *) malloc(info->world_size * sizeof (int));  // TODO free this

            // Calculate the number of elements to receive from each process
            for (int i = 0; i < info->world_size; ++i) {
                recvCounts[i] = partitionResult[3 * i];
            }

            // Calculate the displacements of the elements of each process
            displs[0] = 0;

            for (int i = 1; i < info->world_size; ++i) {
                displs[i] = displs[i - 1] + recvCounts[i - 1];
            }

            // Gather all the elements of the left sub-arrays of all the processes to the gatheredData array
            MPI_Gatherv(
                info->A,
                info->size,
                MPI_INT,
                gatheredData,
                recvCounts,
                displs,
                MPI_INT,
                master_rank,
                communicator
            );

            

            //find the k-th smallest value using the kselect function for all the processes
            *kthSmallest = kselect(gatheredData, left, info->k);
                
            free(partitionResult);
            free(requests);
            free(result->left);
            free(result);
            free(gatheredData);
            free(recvCounts);
            free(displs);
            free(pointsPerProc);

            
            return;

        } else {

            // if the left sub-array is big enough we continue the recursion
            status = 0;

            MPI_Bcast(
                &status,
                1,
                MPI_INT,
                master_rank,
                communicator
            );
        }
        
    } else if(info->k <= left + middle){
        
        // if the k-th smallest value is in the middle we know that it is the pivot
        *kthSmallest = info->pivot;
        
        status = 2;

        // Broadcast to all the slaves to simply return
        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
        
        free(partitionResult);
        free(requests);
        free(result->left);
        free(result->right);
        free(result->middle);
        free(result);
        free(pointsPerProc);
        
        return;

    } else{

        free(info->A);
        free(result->left);
        free(result->middle);

        info->A = result->right;
        info->size = result->rightSize;
        info->k -= left + middle;

        totalPoints = right;

        // fill in pointsPerProc
        for (int i = 0; i < info->world_size; ++i) {
            pointsPerProc[i] = partitionResult[3 * i + 2];
        }

        status = 3;

        MPI_Bcast(
            &status,
            1,
            MPI_INT,
            master_rank,
            communicator
        );
    
        //ending condition
        if(right <= END_RECURSION){
       
            status = 4;

            // Broadcast to all the slaves if they should stop or not
            MPI_Bcast(
                &status,
                1,
                MPI_INT,
                master_rank,
                communicator
            );

            // This array will hold all the elements of the right sub-arrays of all the processes
            int *gatheredData = (int *) malloc(right * sizeof (int));  // TODO free this

            // the number of elements to receive from each process
            int *recvCounts = (int *) malloc(info->world_size * sizeof(int));  // TODO free this

            // the displacements of the elements of each process
            int *displs = (int *) malloc(info->world_size * sizeof(int));  // TODO free this

            // Calculate the number of elements to receive from each process
            for (int i = 0; i < info->world_size; ++i) {
                recvCounts[i] = partitionResult[3 * i + 2];  //was 3 * (i + 1)
            }

            // Calculate the displacements of the elements of each process
            displs[0] = 0;

            for (int i = 1; i < info->world_size; ++i) {
                displs[i] = displs[i - 1] + recvCounts[i - 1];
            }

            // Gather all the elements of the left sub-arrays of all the processes to the gatheredData array
            MPI_Gatherv(
                info->A,
                info->size,
                MPI_INT,
                gatheredData,
                recvCounts,
                displs,
                MPI_INT,
                master_rank,
                communicator
            );

            //find the k-th smallest value using the kselect function for all the processes
            *kthSmallest = kselect(gatheredData, right, info->k);
    

            free(partitionResult);
            free(requests);
            free(result->right);
            free(result);
            free(gatheredData);
            free(recvCounts);
            free(displs);
            free(pointsPerProc);

            
            return;

        } else {
            
            // if the left sub-array is big enough we continue the recursion
            status = 0;

            MPI_Bcast(
                &status,
                1,
                MPI_INT,
                master_rank,
                communicator
            );
        }
    }
    
    free(partitionResult);
    free(requests);


    // balance the number of points that remain in the array of each process
    balancePointsMaster(&info->A, &info->size, totalPoints, info->world_size, min_rank, max_rank, pointsPerProc, communicator);

    free(pointsPerProc);

    // if the remaining array was big enough we continue the recursion
    masterProcess(master_rank, min_rank, max_rank, info, communicator ,kthSmallest);

    free(result);
}

