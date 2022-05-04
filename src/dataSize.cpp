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

    int bufSize = atoi(argv[1]);
    if (procId == ROOT_PROCESS){
        printf("Buffer Size: %d\n", bufSize);
    }
    char *buf = (char *) malloc(bufSize * sizeof(char));
    double initTime = MPI_Wtime();

#ifdef SEND_RECV
    if (procId % 2) {
        // Receive from procID - 1
        MPI_Status status;
        MPI_Recv(buf, bufSize, MPI_CHAR, procId - 1, 0, MPI_COMM_WORLD, &status);
    } else {
        // Send to procID + 1
        MPI_Send(buf, bufSize, MPI_CHAR, procId + 1, 0, MPI_COMM_WORLD);
    }
#else
    MPI_Bcast(buf, bufSize, MPI_CHAR, ROOT_PROCESS, MPI_COMM_WORLD);
#endif
    double endTime = MPI_Wtime();
    if (procId == ROOT_PROCESS)
        printf("Experiment took %lf seconds.\n", endTime - initTime);

    MPI_Finalize();
}