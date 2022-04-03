#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>
// #include <cstring>

int main(int argc, char* argv[]) {
    int opt = 0;
    char *executable = NULL;
    char rankBuffer[5];
    int numProc;
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

    printf("MPIARGS %s %d\n", executable, numProc);
    for (int i=0; i<argc; i++) {
        printf("i=%i argv=%s \n", i, argv[i]);
    }

    // argv[2] = itoa(numProc);

    for (int i=0; i<numProc; i++) {
        int pid = fork();
        if (pid == 0) {
            //parent
            itoa(i, rankBuffer, 10);
            argv[3] = rankBuffer;
            execve(executable, argv+2, NULL);
            break;
        } else {
            //child
            i++;
            if (i == numProc - 1) {
                itoa(i, rankBuffer, 10);
                argv[3] = rankBuffer;
                execve(executable, argv+2, NULL);
                break;
            }
        }
    }
    // execve(executable, argv + 2, NULL);
    // fork();
}