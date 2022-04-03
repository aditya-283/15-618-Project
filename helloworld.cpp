#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>

int main(int argc, char* argv[]) {
    
    // printf("Hello world!\n");
    int nProc=0, procId=0;
    // printf("My %d arguments before init were: ", argc);
    // for (int i=0; i<argc; i++) {
    //     printf("%s ", argv[i]);
    // }
    // printf("\n");

    MPI_Init(&argc, &argv);

    printf("My %d arguments after init are: ", argc);
    for (int i=0; i<argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    MPI_Comm_rank(&procId);
    MPI_Comm_size(&nProc);
    // printf("")
    MPI_Finalize();
}