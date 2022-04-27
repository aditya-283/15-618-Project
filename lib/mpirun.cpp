#include "headers/socket.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>


static void printUsage(void) {
    printf("Usage:\n ./mpirun -n numProcs -e executable [-h]\n");
    printf("-n numProcs   : Number of Processes to create.\n");
    printf("-e executable : Executable to run.\n");
    printf("[-h]          : Print this message.\n");
}



void modifyArg(int argc, char** argv, char** new_argv, int procId, int numProc, int* portIds, char buf[][10]) {    
    int j = 0;
    // Copy existing argv
    for (j = 0; j < argc; j ++) {
        new_argv[j] = argv[j];
    }
    // Copy current process listen ports
    for (; j < (argc + numProc); j ++) {
        snprintf(buf[j - argc], 10, "%d", portIds[(procId * numProc) + (j - argc)]);
        new_argv[j] = buf[j - argc];
    }
    // Copy write ports
    int portOffset = procId;
    for (; j < (argc + 2 * numProc); j ++, portOffset += numProc) {
        snprintf(buf[j - argc], 10, "%d", portIds[portOffset]);
        new_argv[j] = buf[j - argc];
    }

    snprintf(buf[2 * numProc], 10, "%d", procId);
    new_argv[j++] = buf[2 * numProc];

    snprintf(buf[2 * numProc + 1], 10, "%d", numProc);
    new_argv[j++] = buf[2 * numProc + 1];
}


void spawnProcesses(int numProc, int argc, char**argv, char* executable, int* portIds) {
    char buf[(2 * numProc) + 2][10];
    char** new_argv = (char**) malloc((argc + (2 * numProc) + 3) * sizeof(char*));

    for (int procId = 0; procId < numProc; procId ++) {
        int pid = fork();
        if (pid == 0) {
            // parent
            modifyArg(argc, argv, new_argv, procId, numProc, portIds, buf);
            execve(executable, new_argv, NULL);
        } else {
            // child
            if (procId == numProc - 2) {
                modifyArg(argc, argv, new_argv, procId + 1, numProc, portIds, buf);
                execve(executable, new_argv, NULL);
            }
        }
    }
}


int* findPortIds(int numProc) {
    int* portIds = (int*) malloc(numProc * numProc * sizeof(int));
    int testPort = LISTEN_PORT_OFFSET;
    char portString[10];

    for (int i=0; i<numProc * numProc; i++) {
        int fd = -1;
        getPortStr(testPort, portString, 10);
        while ((fd = open_listenfd(portString)) < 0) {
            testPort ++;
        }
        close(fd);
        portIds[i] =  testPort ++;
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