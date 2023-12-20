#ifndef PARTITION_H   
#define PARTITION_H 

#include "structs.h"


PivotedArrays *partition(int* A, int size, int pivot);
int kselect(int* A, int size, int k, int pivot);

#endif