#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;

    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(&procId);
    MPI_Comm_size(&nProc);
    printf("Rank %d   Size %d\n", procId, nProc);
    
    MPI_Finalize();
}