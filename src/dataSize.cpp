#ifdef OUR_MPI
#include "headers/mpi.h"
#else
#include "mpi.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define ROOT_PROCESS 0

// Define which function to benchmark - BCAST or SEND_RECV
#define SEND_RECV


int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    if (argc < 2){
        if (procId == ROOT_PROCESS)
            printf("Error in arguments. Please specify the number of iterations.\nCode Exit.\n");
        MPI_Finalize();
        return 0;
    }
    int N_ITERS = 20;// atoi(argv[1]);
    int dataSizes[] = {32, 256, 1024, 65536, 262144, 1048576, 16777216, 268435456};

    for (int i= 0; i<8; i++) {
        int bufSize = dataSizes[i];
        double initTime = MPI_Wtime();
        char *buf = (char *) malloc(sizeof(char) * bufSize);

        for (int iters = 0; iters < N_ITERS; iters++){
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
        }
        double endTime = MPI_Wtime();
        if (procId == ROOT_PROCESS){
            printf("%d %lf\n", bufSize, (double)(endTime - initTime)/N_ITERS);
        }
    }

    MPI_Finalize();
}