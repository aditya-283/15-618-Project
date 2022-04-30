#include "headers/mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SYNC_SEND_RECEIVE   0
#define BROADCAST           0
#define GATHER              0
#define SCATTER             0
#define ALLGATHER           0
#define REDUCE              0
#define TREEREDUCE          1
#define ALLREDUCE           0
#define ROOT_PROCESS        0

#define ARRAYSIZE 250

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);
    double initTime = MPI_Wtime();
#if SYNC_SEND_RECEIVE
    char buf[100];
    if ((procId % 2) == 0) {
        snprintf(buf, 100, "Hello from Proc %d", procId);
        MPI_Send(buf, strlen(buf)+1, MPI_CHAR, procId + 1, 0, MPI_COMM_WORLD);
        printf("Process %d sent message\n", procId);
    } else {
        MPI_Status status;
        MPI_Recv(buf, 100, MPI_CHAR, procId - 1,  0, MPI_COMM_WORLD, &status);
        printf("Process %d received message: %s\n", procId, buf);
    }
#elif BROADCAST
    char buf[100];
    if (procId == ROOT_PROCESS){
        snprintf(buf, 100, "Hello from proc %d", ROOT_PROCESS);
        MPI_Bcast(buf, strlen(buf)+1, MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);
    } else {
        MPI_Bcast(buf, 100, MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);   
    }
    printf("Process %d - %s\n", procId, buf);

#elif GATHER
    int data = 0;
    if (procId == ROOT_PROCESS) {
        int buffer[nProc];
        data = 1000 + procId;
        MPI_Gather(&data, 1, MPI_INT, buffer, 1, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
        printf("Root Process %d: ", procId);
        for (int i = 0; i < nProc; i ++){
            printf("%d ", buffer[i]);
        }
        printf("\n");
    } else {
        data = 1000 + procId;
        printf("Process %d: Sending %d\n", procId, data);
        MPI_Gather(&data, 1, MPI_INT, NULL, 0, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    }
#elif SCATTER
    int len = 10;
    int recvBuffer[len];

    if (procId == ROOT_PROCESS) {
        int sendBuffer[len * nProc];
        
        printf("Root Process %d: ", procId);
        for (int i = 0; i < len * nProc; i ++) {
            sendBuffer[i] = 1000 + i;
            printf("%d ", sendBuffer[i]);
        }
        printf("\n");

        MPI_Scatter(sendBuffer, len, MPI_INT, recvBuffer, len, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    } else {
        MPI_Scatter(NULL, 0, MPI_INT, recvBuffer, len, MPI_INT, ROOT_PROCESS, MPI_COMM_WORLD);
    }

    printf("Process %d: ", procId);
    for(int i = 0; i < len; i ++) {
        printf("%d ", recvBuffer[i]);
    }
    printf("\n");

#elif ALLGATHER
    int buffer[nProc];
    memset(buffer, 0, nProc * sizeof(int));
    int data = 1000 + procId;
    MPI_Allgather(&data, 1, MPI_INT, buffer, 1, MPI_INT, MPI_COMM_WORLD);
    if (procId == nProc - 1) {
        printf("Process %d: ", procId);
        for (int i = 0; i < nProc; i ++){
            printf("%d ", buffer[i]);
        }
        printf("\n");
    }
#elif REDUCE
    int* output = (int*)malloc(ARRAYSIZE * sizeof(int));
    int* data = (int*)malloc(ARRAYSIZE * sizeof(int));
    for (int i=0; i<ARRAYSIZE; i++) data[i] = procId;
    MPI_Reduce(data, output, ARRAYSIZE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (procId == 0) {
        printf("Process %d: Reduced value: %d\n", procId, output[ARRAYSIZE-1]);
    }
#elif TREEREDUCE
    int* output = (int*)malloc(ARRAYSIZE * sizeof(int));
    int* data = (int*)malloc(ARRAYSIZE * sizeof(int));
    for (int i=0; i<ARRAYSIZE; i++) data[i] = procId;
    MPI_treereduce(data, output, ARRAYSIZE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (procId == 0) {
        printf("Process %d: Reduced value: %d\n", procId, output[ARRAYSIZE-1]);
    }
#elif ALLREDUCE
    int output[ARRAYSIZE];
    int data[ARRAYSIZE];
    for (int i=0; i<ARRAYSIZE; i++) data[i] = procId;
    MPI_Allreduce(data, output, ARRAYSIZE, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (procId == 125) {
        printf("Process %d: Reduced value: %d\n", procId, output[ARRAYSIZE-1]);
    }

#endif
    double endTime = MPI_Wtime();
    if (procId == 0)
        printf("Experiment took %lf seconds.\n", endTime - initTime);
    MPI_Finalize();
}