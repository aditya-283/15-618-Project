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
            printf("MPI.CPP  Process %d: Error in %s() - ", processId, funcName);\
            printf(__VA_ARGS__);                                        \
            printf("\n");                                               \
        }

#define printMessage(...);                                              \
        if (PRINT_MESSAGES) {                                           \
            printf("MPI.CPP  Process %d: ", processId);                 \
            printf(__VA_ARGS__);                                        \
            printf("\n");                                               \
        }


typedef struct {
    int proc;
    int count;
    MPI_Datatype datatype;
    void *buf;
} threadArgs_t;

static int numProcess;
static int processId;
static int *listenfd;
static struct sockaddr_storage clientaddr;

static inline int getPortId (int procId, int portOffset) {
    return LISTEN_PORT_OFFSET + (numProcess * procId) + portOffset;
}

static inline void getPortStr(int procId, int portOffset, char* listenPortStr, int size) {
    snprintf(listenPortStr, size, "%d", getPortId(procId, portOffset));
}

int MPI_Init(int *argc, char*** argv) {
    numProcess = atoi(*argv[0]);
    processId = atoi((*argv)[1]);
    *argc = *argc - 2;
    *argv = *argv + 2;

    char listenPortStr[10];
    listenfd = (int *) malloc(sizeof(int) * numProcess);
    for (int index = 0; index < numProcess; index ++) {
        getPortStr(processId, index, listenPortStr, 10);
        listenfd[index] = open_listenfd(listenPortStr);
        if (listenfd[index] < 0) {
            printError("MPI_Init", "open_listenfd: FD %d returned %d with errno %d", index, listenfd[index], errno);
            return -1;
        }
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
    for (int index = 0; index < numProcess; index ++) {
        if (close(listenfd[index])) {
            printError("MPI_Finalize", "Unable to close listenfd %d.", index);
        }
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
    getPortStr(dest, processId, destPortStr, 10);
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
    sourceFd = accept(listenfd[source], (struct sockaddr*)&clientaddr, &clientlen);
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

static void *sendBcastData(void *varg)
{
    threadArgs_t *threadArg = (threadArgs_t *) varg;
    MPI_Send(threadArg->buf, threadArg->count, threadArg->datatype, threadArg->proc);
    free(threadArg);
    return 0;
}

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root) {
    if (processId == root) {
        // Send to all the processes
        pthread_t threadId[numProcess];
        for (int destProc = 0; destProc < numProcess; destProc ++) {
            if (destProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->proc = destProc;
            threadArg->count = count;
            threadArg->datatype = datatype;
            threadArg->buf = buf;
            pthread_create(&threadId[destProc], NULL, sendBcastData, (void *) threadArg);
        }
        for (int i = 0; i < numProcess; i ++) {
            if (i == root) continue;
            pthread_join(threadId[i], NULL);
        }
    } else {
        // Receive from root process
        MPI_Status status;
        MPI_Recv(buf, count, datatype, root, &status);
    }
    return 0;
}


void *recvGatherData(void *varg)
{
    threadArgs_t *threadArg = (threadArgs_t *) varg;
    MPI_Status status;
    MPI_Recv(threadArg->buf, threadArg->count, threadArg->datatype, threadArg->proc, &status);
    free(threadArg);
    return 0;
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root) {
    if (processId == root) {
        // Receive data from other processes
        pthread_t threadId[numProcess];
        for (int sourceProc = 0; sourceProc < numProcess; sourceProc ++) {
            if (sourceProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->proc = sourceProc;
            threadArg->count = recvcount;
            threadArg->datatype = recvtype;
            threadArg->buf = (void*) ((char *) recvbuf + (recvcount * recvtype * sourceProc));
            pthread_create(&threadId[sourceProc], NULL, recvGatherData, (void *) threadArg);
        }
        // Could still do this memcpy in another separate thread
        void *destBuf = (void *)((char *) recvbuf + (recvcount * recvtype * root));
        memcpy(destBuf, sendbuf, sendcount * sendtype);

        for (int i = 0; i < numProcess; i ++) {
            if (i == root) continue;
            pthread_join(threadId[i], NULL);
        }
    } else {
        // Send data to root process
        MPI_Send(sendbuf, sendcount, sendtype, root);
    }
    return 0;
}