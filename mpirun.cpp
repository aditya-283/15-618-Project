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



void modifyArg(int argc, char** argv, char** new_argv, int numProc, int* portIds, char buf[][10]) {
    for (int j=0; j<argc + numProc*numProc; j++) {
        if (j < argc) {
            new_argv[j] = argv[j];
        } else {
            snprintf(buf[j], 10, "%d", portIds[(j - argc)]);
            new_argv[j] = buf[j];
        }
    }

    
    snprintf(buf[argc + numProc*numProc], 10, "%d", numProc);
    new_argv[argc + numProc] = buf[argc + numProc + 1];
}


void spawnProcesses(int numProc, int argc, char**argv, char* executable, int* portIds) {
    char** new_argv = (char**)malloc((argc + numProc*numProc + 2) * sizeof(char*));
    char buf[numProc*numProc+2][10];
    modifyArg(argc, argv, new_argv, numProc, portIds, buf);
    for (int i=0; i<numProc; i++) {
        int pid = fork();
        if (pid == 0) {
            // parent
            snprintf(buf[argc + numProc*numProc+1], 10, "%d", i);
            new_argv[argc+numProc*numProc+1] = buf[argc+numProc];
            execve(executable, new_argv, NULL);
            break;
        } else {
            // child
            if (i == numProc - 2) {
                snprintf(buf[argc + numProc*numProc + 1], 10, "%d", i + 1);
                new_argv[argc + numProc*numProc + 1] = buf[argc+numProc];
                execve(executable, new_argv, NULL);
                break;
            }   
        }
    }
}


   
int* findPortIds(int numProc) {
    int* portIds = (int*)malloc(numProc * numProc * sizeof(int));
    for (int i=0; i<numProc * numProc; i++) {
        portIds[i] = 3000 + i;
    }
    return portIds;
}


int main(int argc, char* argv[]) {
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

    int* portIds = findPortIds(numProc);
    spawnProcesses(numProc, argc, argv, executable, portIds);

    return 0;
}