#ifndef SLAVEPROCESS_H   /* Include guard */
#define SLAVEPROCESS_H 

#include <mpi.h>
#include "structs.h"


void slaveProcess(int master_rank, int min_rank, int max_rank, Info *info, MPI_Comm communicator);

#endif // MASTER_H