#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>
// #include <cstring>


int main(int argc, char* argv[]) {

    // check argument order - diff orders allowed?
    // diff option names
    // check  numProc <= max number of processors
    // CPU affinity 
    // figure out errno, and return values for errors

    int opt = 0;
    char *executable = NULL;
    char rankBuffer[100];
    int numProc=0;
    do {
        opt = getopt(argc, argv, "e:n:");
        switch (opt) {
        case 'e':
            executable = optarg;
            break;
        case 'n':
            numProc = atoi(optarg);
            break;
        }
    } while (opt != -1);

    // printf("MPIARGS %s %d\n", executable, numProc);
    // for (int i=0; i<argc; i++) {
    //     printf("i=%i argv=%s \n", i, argv[i]);
    // }

    // argv[2] = itoa(numProc);

    for (int i=0; i<numProc; i++) {
        int pid = fork();
        if (pid == 0) {
            //parent
            snprintf(rankBuffer, sizeof(rankBuffer), "%d", i);
            argv[3] = rankBuffer;
            execve(executable, argv+2, NULL);
            break;
        } else {
            //child
            if (i == numProc - 2) {
                snprintf(rankBuffer, sizeof(rankBuffer), "%d", i + 1);
                argv[3] = rankBuffer;
                execve(executable, argv+2, NULL);
                break;
            }
        }
    }
    // execve(executable, argv + 2, NULL);
    // fork();
}