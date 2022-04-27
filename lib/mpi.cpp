#include "headers/mpi.h"
#include "headers/socket.h"
#include "headers/rio.h"
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
            printf("MPI.CPP  Process %d: Error in %s() - ", procId, funcName);\
            printf(__VA_ARGS__);                                        \
            printf("\n");                                               \
        }

#define printMessage(...);                                              \
        if (PRINT_MESSAGES) {                                           \
            printf("MPI.CPP  Process %d: ", procId);                 \
            printf(__VA_ARGS__);                                        \
            printf("\n");                                               \
        }


typedef struct {
    int proc;
    int count;
    MPI_Datatype datatype;
    void *buf;
} threadArgs_t;

static int numProc, procId;
static int *listenfd;
static char **listenPortStrings, **writePortStrings;
static struct sockaddr_storage clientaddr;

int MPI_Init(int *argc, char*** argv) {
    numProc = atoi((*argv)[--(*argc)]);
    procId = atoi((*argv)[--(*argc)]);

    listenPortStrings = (char**) malloc(numProc * sizeof(char*));
    for (int i = 0; i < numProc; i++) {
        listenPortStrings[i] = (*argv)[*argc - 2 * numProc + i];
    }

    writePortStrings = (char **) malloc(numProc * sizeof(char*));
    for (int i = 0; i < numProc; i++) {
        writePortStrings[i] = (*argv)[*argc - numProc + i];
    }    

    *argc = *argc - 2 * numProc - 4;
    *argv = *argv + 4;

    listenfd = (int*) malloc(sizeof(int) * numProc);
    for (int index = 0; index < numProc; index ++) {
        if (index == procId) continue;
        listenfd[index] = open_listenfd(listenPortStrings[index]);
        if (listenfd[index] < 0) {
            printError("MPI_Init", "open_listenfd: FD %d returned %d with errno %d", index, listenfd[index], errno);
            return -1;
        }
    }
    return 0;
}

int MPI_Comm_rank(MPI_Comm comm, int* rank) {
    *rank = procId;
    return 0;
}

int MPI_Comm_size(MPI_Comm comm, int* size) {
    *size = numProc;
    return 0;
}

int MPI_Finalize(void) {
    for (int index = 0; index < numProc; index ++) {
        if (close(listenfd[index])) {
            printError("MPI_Finalize", "Unable to close listenfd %d.", index);
        }
    }
    // Sync with all process somehow
    // exit(0);
    return 0;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    int destFd = -1;
    ssize_t bytes_transacted = 0;
    
    // Open Connection with the destination process
    while (destFd < 0) {
        destFd = open_clientfd("localhost", writePortStrings[dest]);
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

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
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

static void *sendBcastData(void *varg) {
    threadArgs_t *threadArg = (threadArgs_t *) varg;
    MPI_Send(threadArg->buf, threadArg->count, threadArg->datatype, threadArg->proc, 0, MPI_COMM_WORLD);
    free(threadArg);
    return 0;
}

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    if (procId == root) {
        // Send to all the processes
        pthread_t threadId[numProc];
        for (int destProc = 0; destProc < numProc; destProc ++) {
            if (destProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->proc = destProc;
            threadArg->count = count;
            threadArg->datatype = datatype;
            threadArg->buf = buf;
            pthread_create(&threadId[destProc], NULL, sendBcastData, (void *) threadArg);
        }
        for (int i = 0; i < numProc; i ++) {
            if (i == root) continue;
            pthread_join(threadId[i], NULL);
        }
    } else {
        // Receive from root process
        MPI_Status status;
        MPI_Recv(buf, count, datatype, root, 0, MPI_COMM_WORLD, &status);
    }
    return 0;
}


static void *recvGatherData(void *varg) {
    threadArgs_t *threadArg = (threadArgs_t *) varg;
    MPI_Status status;
    MPI_Recv(threadArg->buf, threadArg->count, threadArg->datatype, threadArg->proc, 0, MPI_COMM_WORLD, &status);
    free(threadArg);
    return 0;
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    if (procId == root) {
        // Receive data from other processes
        pthread_t threadId[numProc];
        for (int sourceProc = 0; sourceProc < numProc; sourceProc ++) {
            if (sourceProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->proc = sourceProc;
            threadArg->count = recvcount;
            threadArg->datatype = recvtype;
            threadArg->buf = (void*) ((char *) recvbuf + (recvcount * recvtype * sourceProc));
            pthread_create(&threadId[sourceProc], NULL, recvGatherData, (void *) threadArg);
        }
        void *destBuf = (void *)((char *) recvbuf + (recvcount * recvtype * root));
        memcpy(destBuf, sendbuf, sendcount * sendtype);

        for (int i = 0; i < numProc; i ++) {
            if (i == root) continue;
            pthread_join(threadId[i], NULL);
        }
    } else {
        // Send data to root process
        MPI_Send(sendbuf, sendcount, sendtype, root, 0, MPI_COMM_WORLD);
    }
    return 0;
}


int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, 0, comm);
    MPI_Bcast(recvbuf, recvcount * numProc, recvtype, 0, comm);
    return 0;
}