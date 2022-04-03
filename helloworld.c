#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <unistd.h>

int main(int argc, char* argv[]) {
    
    // printf("Hello world!\n");
    printf("My %d arguments are: ", argc);

    for (int i=0; i<argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
}