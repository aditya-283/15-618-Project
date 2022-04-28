#include "headers/socket.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>


static void printUsage(void) {
    printf("Usage:\n./mpirun [-h] -n numProcs -e executable [args]\n");
    printf("[-h]          : Print this message.\n");
    printf("-n numProcs   : Number of Processes to create.\n");
    printf("-e executable : Executable to run.\n");
    printf("[args]        : Arguments for executable.\n");
    printf("\n");
}



char** getNewArgv(int argc, char** argv, int procId, int numProc, int* portIds) {    
    int j = 0;
    char** newArgv = (char**) malloc((argc + (2 * numProc) + 3) * sizeof(char*));

    // Copy existing argv
    for (j = 0; j < argc; j ++) {
        newArgv[j] = argv[j];
    }
    // Copy current process listen ports
    for (; j < (argc + numProc); j ++) {
        newArgv[j] = (char *) malloc(10 * sizeof(char));
        snprintf(newArgv[j], 10, "%d", portIds[(procId * numProc) + (j - argc)]);
    }
    // Copy write ports
    int portOffset = procId;
    for (; j < (argc + 2 * numProc); j ++, portOffset += numProc) {
        newArgv[j] = (char *) malloc(10 * sizeof(char));
        snprintf(newArgv[j], 10, "%d", portIds[portOffset]);
    }

    // Process ID
    newArgv[j] = (char *) malloc(10 * sizeof(char));
    snprintf(newArgv[j++], 10, "%d", procId);

    // Number of Process
    newArgv[j] = (char *) malloc(10 * sizeof(char));
    snprintf(newArgv[j++], 10, "%d", numProc);

    newArgv[j++] = NULL;

    return newArgv;
}


void spawnProcesses(int numProc, int argc, char**argv, char* executable, int* portIds) {
    char** newArgv;

    for (int procId = 0; procId < numProc; procId ++) {
        int pid = fork();
        if (pid == 0) {
            // parent
            newArgv = getNewArgv(argc, argv, procId, numProc, portIds);
            execve(executable, newArgv, NULL);
        } else {
            // child
            if (procId == numProc - 2) {
                newArgv = getNewArgv(argc, argv, procId + 1, numProc, portIds);
                execve(executable, newArgv, NULL);
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
    int numProc = 0;
    bool nArgPresent = false, eArgPresent = false;
    int opt = 0, getopt_argc = 0;
    
    // Process the command line arguments
    getopt_argc = (argc < 5) ? argc : 5;
    opterr = 0; // Disable getopt error messages on stderr
    do {
        opt = getopt(getopt_argc, argv, "n:e:h");
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
            return 0;
            break;
        }
    } while (opt != -1);

    // Print Usage Message if arguments are incorrect
    if ((optind < 5) || !(nArgPresent && eArgPresent)) {
        printf("Incorrect arguments!\n");
        printUsage();
        return -1;
    }

    // Check if the executable is valid or not
    if (access(executable, F_OK | X_OK)) {
        printf("mpirun: %s either does not exist or not an executable!\n", executable);
        return -1;
    }

    // Get valid port IDs for inter-process communication
    int* portIds = findPortIds(numProc);
    // Spawn the processes
    spawnProcesses(numProc, argc - 4, argv + 4, executable, portIds);

    return 0;
}