#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>
#include <string.h>
#include <chrono>

#define ROOT_PROC       0
#define ARRAY_SIZE      1000000

int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);
    
    using namespace std::chrono;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;
    auto initStart = Clock::now();

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

        auto compComplete = Clock::now();
        printf("Final Average is %lf\n", finalAvg);
        printf("Computation took %lf seconds\n", duration_cast<dsec>(compComplete - initStart).count());

    }


    MPI_Finalize();
}