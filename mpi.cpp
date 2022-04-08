#include "mpi.h"
#include "socket.h"
#include "rio.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h> /* struct sockaddr */
#include <sys/types.h>  /* struct sockaddr */
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>

#define PRINT_ERRORS 1
#define PRINT_MESSAGES 1

#define printError(funcName, ...);                                      \
        if (PRINT_ERRORS) {                                             \
            printf("Process %d: Error in %s() - ", processId, funcName);\
            printf(__VA_ARGS__);                                        \
            printf("\n");                                               \
        }

#define printMessage(...);                                              \
        if (PRINT_MESSAGES) {                                           \
            printf("Process %d: ", processId);                          \
            printf(__VA_ARGS__);                                        \
            printf("\n");                                               \
        }


typedef struct {
    int destProc;
    int count;
    MPI_Datatype datatype;
    void *buf;
} threadArgs_t;

static int numProcess;
static int processId;
static int listenfd = -1;
static struct sockaddr_storage clientaddr;

static inline int getPortId (int procId) {
    return LISTEN_PORT_OFFSET + procId;
}

static inline void getPortString(int procId, char* listenPortStr, int size) {
    snprintf(listenPortStr, size, "%d", getPortId(procId));
}

int MPI_Init(int *argc, char*** argv) {
    numProcess = atoi(*argv[0]);
    processId = atoi((*argv)[1]);
    *argc = *argc - 2;
    *argv = *argv + 2;

    char listenPortStr[10];
    getPortString(processId, listenPortStr, 10);
    listenfd = open_listenfd(listenPortStr);
    if (listenfd < 0) {
        printError("MPI_Init", "open_listenfd: returned %d with errno %d", listenfd, errno);
        return -1;
    }
    return 0;
}

int MPI_Comm_rank(int* procId) {
    *procId = processId;
    return 0;
}

int MPI_Comm_size(int* numProc) {
    *numProc = numProcess;
    return 0;
}

int MPI_Finalize(void) {
    if (close(listenfd)) {
        printError("MPI_Finalize", "Unable to close listenfd.");
    }
    // Sync with all process somehow
    // exit(0);
    return 0;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest) {
    int destFd = -1;
    char destPortStr[10];
    ssize_t bytes_transacted = 0;
    
    // Open Connection with the destination process
    getPortString(dest, destPortStr, 10);
    while (destFd < 0) {
        destFd = open_clientfd("localhost", destPortStr);
    }

    // Send the data to the destination process
    bytes_transacted = rio_writen(destFd, buf, count * datatype);
    if (bytes_transacted < (ssize_t)(count * datatype)) {
        printError("MPI_Send", "Short write detected!");
        close(destFd);
        return -1;
    }
    close(destFd);

    return 0;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, MPI_Status *status) {
    socklen_t clientlen;
    int sourceFd = -1;
    rio_t sourceRio;
    ssize_t bytes_transacted;
    ssize_t readUntilNow = 0;
     
    // Open Connection with the source process
    clientlen = sizeof(struct sockaddr_storage);
    sourceFd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
    if (sourceFd < 0) {
        printError("MPI_Recv", "Accept Failed.");
        return -1;
    }
    
    // Read the data and save it into the buffer
    rio_readinitb(&sourceRio, sourceFd);
    while ((bytes_transacted = rio_readnb(&sourceRio, (char *)buf + readUntilNow, count * datatype))) {
        if (readUntilNow > (ssize_t)(count * datatype)) break;
        readUntilNow += bytes_transacted;
    }
    close(sourceFd);

    return 0;
}

void *sendBcastData(void *varg)
{
    threadArgs_t *threadArg = (threadArgs_t *) varg;
    int destProc = threadArg->destProc;
    int count = threadArg->count;
    MPI_Datatype datatype = threadArg->datatype;
    void *buf = threadArg->buf;
    
    // pthread_detach(pthread_self());
    MPI_Send(buf, count, datatype, destProc);
    printMessage("Finished Sending the data to %d", destProc);

    free(threadArg);
    pthread_exit(NULL);
}

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root) {
    if (processId == root) {
        // Send to all the processes
        pthread_t threadId;
        for (int destProc = 0; destProc < numProcess; destProc ++) {
            if (destProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->destProc = destProc;
            threadArg->count = count;
            threadArg->datatype = datatype;
            threadArg->buf = buf;
            pthread_create(&threadId, NULL, sendBcastData, (void *) threadArg);
            pthread_join(threadId, NULL);
        }
    } else {
        // Receive from root process
        MPI_Status status;
        printMessage("Starting to receive Data from Root");
        MPI_Recv(buf, count, datatype, root, &status);
    }
    return 0;
}