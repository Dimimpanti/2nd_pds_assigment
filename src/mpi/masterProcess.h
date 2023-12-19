#ifndef MASTERPROCESS_H   /* Include guard */
#define MASTERPROCESS_H 

#include "structs.h"


void masterProcess(int master_rank, int min_rank, int max_rank, Info *info, MPI_Comm communicator);

#endif // MASTER_H