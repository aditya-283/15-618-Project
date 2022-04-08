#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>


static void printUsage(void) {
    printf("Usage:\n ./mpirun -n numProcs -e executable [-h]\n");
    printf("-n numProcs   : Number of Processes to create.\n");
    printf("-e executable : Executable to run.\n");
    printf("[-h]          : Print this message.\n");
}


int main(int argc, char* argv[]) {
    char rankBuffer[10];
    char *executable = NULL;
    bool nArgPresent = false, eArgPresent = false;
    int numProc = 0, opt = 0;

    opterr = 0; // Disable getopt error messages on stderr
    do {
        opt = getopt(argc, argv, "n:e:h");
        switch (opt) {
        case 'n':
            numProc = atoi(optarg);
            nArgPresent = true;
            break;
        case 'e':
            executable = optarg;
            eArgPresent = true;
            break;
        case 'h':
            printUsage();
            break;
        }
    } while (opt != -1);

    // Print Usage Message if arguments are incorrect
    if ((optind < 5) || !(nArgPresent && eArgPresent)) {
        printUsage();
        return 0;
    }

    for (int i=0; i<numProc; i++) {
        int pid = fork();
        if (pid == 0) {
            // parent
            snprintf(rankBuffer, sizeof(rankBuffer), "%d", i);
            argv[3] = rankBuffer;
            execve(executable, argv+2, NULL);
            break;
        } else {
            // child
            if (i == numProc - 2) {
                snprintf(rankBuffer, sizeof(rankBuffer), "%d", i + 1);
                argv[3] = rankBuffer;;
                execve(executable, argv+2, NULL);
                break;
            }   
        }
    }

    return 0;
}