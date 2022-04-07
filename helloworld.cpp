#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(&procId);
    MPI_Comm_size(&nProc);

    char buf[100];
    if ((procId % 2) == 0) {
        snprintf(buf, 100, "Hello from Proc %d", procId);
        MPI_Send(buf, strlen(buf), MPI_CHAR, procId + 1);
        printf("Process %d sent message\n", procId);
    } else {
        MPI_Status status;
        MPI_Recv(buf, 100, MPI_CHAR, procId - 1, &status);
        printf("Process %d received message: %s\n", procId, buf);
    }

    MPI_Finalize();
}