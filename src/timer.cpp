#ifdef OUR_MPI
#include "headers/mpi.h"
#else
#include "mpi.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define ROOT_PROCESS 0

// Define which function to benchmark - BCAST or SEND_RECV
#define BCAST


int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    for (int i = 0; i < 100000000; i ++);

    MPI_Finalize();
}