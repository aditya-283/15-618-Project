#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    printf("Before init\n");
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(&procId);
    MPI_Comm_size(&nProc);
    printf("Rank %d   Size %d\n", procId, nProc);
    
    if (procId == 2) {
        char sendBuf[] = "hello from process 2";
        MPI_Send(sendBuf, sizeof(sendBuf), MPI_CHAR, 3);
        printf("Sending successful %s\n", sendBuf);
    } else if (procId == 3) {
        char recvBuf[100];
        MPI_Status status;
        MPI_Recv(recvBuf, 50, MPI_CHAR, 2, &status);
        printf("Receive succesful %s\n", recvBuf);
    }
    MPI_Finalize();
}