#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ROOT_PROC       0
#define ARRAY_SIZE      1000000000

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    double initStart = MPI_Wtime();

    long int chunkSize = ARRAY_SIZE / nProc;
    int *intArray = (int *) malloc(sizeof(int) * chunkSize);

    for (int i = 0; i < chunkSize; i ++) {
        intArray[i] = rand() % 100;
    }

    long int sum = 0;
    for (int i = 0; i < chunkSize; i ++) {
        sum += intArray[i];
    }    
    double localAvg = (double) sum / chunkSize;

    if (procId != ROOT_PROC) {
        MPI_Send(&localAvg, 1, MPI_DOUBLE, ROOT_PROC, 0, MPI_COMM_WORLD);
    } else {
        double *avgArray = (double *) malloc(sizeof(double) * nProc);
        avgArray[ROOT_PROC] = localAvg;
        MPI_Status status;
        for (int sourceProc = 0; sourceProc < nProc; sourceProc ++) {
            if (sourceProc == ROOT_PROC) continue;
            MPI_Recv(&avgArray[sourceProc], 1, MPI_DOUBLE, sourceProc, 0, MPI_COMM_WORLD, &status);
        }

        double finalAvg = 0;
        for (int i = 0; i < nProc; i ++) {
            finalAvg += avgArray[i];
        }
        finalAvg /= nProc;

        double compComplete = MPI_Wtime();
        printf("Final Average is %lf\n", finalAvg);
        printf("Computation took %lf seconds\n", (compComplete - initStart));

    }


    MPI_Finalize();
}