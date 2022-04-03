#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>
#include <errno.h>
static int nProc;
static int pId;

int MPI_Init(int *argc, char*** argv) {
    // nProc = atoi(*argv[0]);
    // pId = atoi(*argv[1]);
    *argc = *argc - 2;
    printf("%p\n", *argv[1]);
    *argv = *argv + 2;
    return 0;
}

int MPI_Comm_rank(int* procId) {
    *procId = pId;
    return 0;
}

int MPI_Comm_size(int* numProc) {
    *numProc = nProc;
    return 0;
}

int MPI_Finalize(void) {
    printf("Done for the day!\n");
    return 0;
}