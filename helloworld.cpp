#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>
#include <string.h>

#define SYNC_SEND_RECEIVE   0
#define BROADCAST           0
#define GATHER              1

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(&procId);
    MPI_Comm_size(&nProc);

#if SYNC_SEND_RECEIVE
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
#elif BROADCAST
    char buf[100];
    if (procId == 0){
        snprintf(buf, 100, "Hello from proc 0");
        MPI_Bcast(buf, strlen(buf)+1, MPI_CHAR, 0);
    } else {
        MPI_Bcast(buf, 100, MPI_CHAR, 0);   
    }
    printf("Process %d - %s\n", procId, buf);

#elif GATHER
    int data = 0;
    if (procId == 0) {
        int buffer[nProc];
        data = 1000 + procId;
        MPI_Gather(&data, 1, MPI_INT, buffer, 1, MPI_INT, 0);
        printf("Root Process %d: ", procId);
        for (int i = 0; i < nProc; i ++){
            printf("%d ", buffer[i]);
        }
        printf("\n");
    } else {
        data = 1000 + procId;
        printf("Process %d: Sending %d\n", procId, data);
        MPI_Gather(&data, 1, MPI_INT, NULL, 0, MPI_INT, 0);
    }
#endif

    MPI_Finalize();
}