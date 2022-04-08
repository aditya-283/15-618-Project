#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>
#include <string.h>

#define SYNC_SEND_RECEIVE 0
#define BROADCAST 1

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(&procId);
    MPI_Comm_size(&nProc);

#if (SYNC_SEND_RECEIVE)
    char buf[100];
    if ((procId % 2) == 0) {
        snprintf(buf, 100, "Hello from Proc %d", procId);
        MPI_Send(buf, strlen(buf)+1, MPI_CHAR, procId + 1);
        printf("Process %d sent message\n", procId);
    } else {
        MPI_Status status;
        MPI_Recv(buf, 100, MPI_CHAR, procId - 1, &status);
        printf("Process %d received message: %s\n", procId, buf);
    }
#elif (BROADCAST)
    char buf[100];
    if (procId == 0){
        snprintf(buf, 100, "Hello from proc 0");
        MPI_Bcast(buf, strlen(buf)+1, MPI_CHAR, 0);
    } else {
        MPI_Bcast(buf, 100, MPI_CHAR, 0);   
    }
    printf("Process %d - %s\n", procId, buf);

#endif

    MPI_Finalize();
}