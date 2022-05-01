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
#include <chrono>
#include <algorithm>
#include <fcntl.h>

#define PARENT_PROCESS  (numProc - 1)
#define PRINT_ERRORS 1
#define PRINT_MESSAGES 1

#define FAPPLY(f, a, b, t) *(t*)a = f(*(t*)a, *(t*)b)
#define OPAPPLY(op, a, b, t) *(t*)a = *(t*)a op *(t*)b

#define PERFORMF(f, a, b, t) if (t == MPI_INT) FAPPLY(f, a, b, int);                \
                            else if (t == MPI_FLOAT) FAPPLY(f, a, b, float);        \
                            else if (t == MPI_LONG) FAPPLY(f, a, b, long);          \
                            else if (t == MPI_DOUBLE) FAPPLY(f, a, b, double);  

#define PERFORMOP(op, a, b, t) if (t == MPI_INT) OPAPPLY(op, a, b, int);            \
                             else if (t == MPI_FLOAT) OPAPPLY(op, a, b, float);     \
                             else if (t == MPI_LONG) OPAPPLY(op, a, b, long);       \
                             else if (t == MPI_DOUBLE) OPAPPLY(op, a, b, double);

#define printError(funcName, ...);                                              \
        if (PRINT_ERRORS) {                                                     \
            printf("MPI.CPP  Process %d: Error in %s() - ", procId, funcName);  \
            printf(__VA_ARGS__);                                                \
            printf("\n");                                                       \
        }

#define printMessage(...);                                                      \
        if (PRINT_MESSAGES) {                                                   \
            printf("MPI.CPP  Process %d: ", procId);                            \
            printf(__VA_ARGS__);                                                \
            printf("\n");                                                       \
        }

typedef struct {
    int proc;
    int count;
    MPI_Datatype datatype;
    void *buf;
} threadArgs_t;

static int commonPgid;
static int numProc, procId;
static int *listenfd;
static char **listenPortStrings, **writePortStrings;
static struct sockaddr_storage clientaddr;



typedef void handler_t(int);


// Safe, wrapped sigaction
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) {
        perror("Signal error");
        exit(1);
    }

    return old_action.sa_handler;
}

int isValidFd(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

void sigintHandler(int sig) {
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);

    if (procId == PARENT_PROCESS) {
        kill(-1 * commonPgid, SIGINT);
    }
    MPI_Finalize(); // reap all children, which closes all open ports

    sigprocmask(SIG_SETMASK, &prev_all, NULL);
    errno = olderrno;
    exit(-1);
}

int MPI_Init(int *argc, char*** argv) {
    sscanf((*argv)[--(*argc)], "PGID=%d", &commonPgid);
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

    *argc = *argc - 2 * numProc;

    listenfd = (int*) malloc(sizeof(int) * numProc);
    for (int index = 0; index < numProc; index ++) {
        if (index == procId) continue;
        listenfd[index] = open_listenfd(listenPortStrings[index]);
        if (listenfd[index] < 0) {
            printError("MPI_Init", "open_listenfd: FD %d returned %d with errno %d", index, listenfd[index], errno);
            return -1;
        }
    }
    Signal(SIGINT, sigintHandler); // Handles Ctrl-C
    Signal(SIGCHLD, sigintHandler); // Handles children dying
    Signal(SIGTSTP, sigintHandler); // Handles Ctrl-Z
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

int msizeof(MPI_Datatype type) {
    switch (type) {
        case MPI_CHAR:
            return sizeof(char);
        case MPI_INT:
            return sizeof(int);
        case MPI_LONG:
            return sizeof(long);
        case MPI_FLOAT:
            return sizeof(float);
        case MPI_DOUBLE:
            return sizeof(double);
        default:
            printError("msizeof", "No such datatype %d", type);
            return -1;
    }
}

int MPI_Finalize(void) {
    if (procId == PARENT_PROCESS) {
        printMessage("Cleaning up before exit ...\n");
    }
    for (int index = 0; index < numProc; index ++) {
        if (isValidFd(listenfd[index]) && close(listenfd[index]) < 0) {
            printMessage("Closing failed for fd %d!", listenfd[index]);
        } else {
            // printMessage("Closing successful for fd %d!", listenfd[index]);
        }
    }

    // Wait for all child process to reap
    if (procId == PARENT_PROCESS) {
        for (int proc = 0 ; proc < numProc-1; proc ++) {
            wait(NULL);
        }
    }
    return 0;
}

int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    int destFd = -1;
    ssize_t bytes_transacted = 0;
    
    // Open Connection with the destination process
    while (destFd < 0) {
        destFd = open_clientfd("localhost", writePortStrings[dest]);
    }

    // Send the data to the destination process
    bytes_transacted = rio_writen(destFd, buf, count * msizeof(datatype));
    if (bytes_transacted < (ssize_t)(count * msizeof(datatype))) {
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
        printError("MPI_Recv", "Accept Failed connecting to Process %d", source);
        return -1;
    }
    
    // Read the data and save it into the buffer
    rio_readinitb(&sourceRio, sourceFd);
    while ((bytes_transacted = rio_readnb(&sourceRio, (char *)buf + readUntilNow, count * msizeof(datatype)))) {
        if (readUntilNow > (ssize_t)(count * msizeof(datatype))) break;
        readUntilNow += bytes_transacted;
    }
    close(sourceFd);

    return 0;
}

static void *rootSendData(void *varg) {
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
            pthread_create(&threadId[destProc], NULL, rootSendData, (void *) threadArg);
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


static void *rootReceiveData(void *varg) {
    threadArgs_t *threadArg = (threadArgs_t *) varg;
    MPI_Status status;
    MPI_Recv(threadArg->buf, threadArg->count, threadArg->datatype, threadArg->proc, 0, MPI_COMM_WORLD, &status);
    free(threadArg);
    return 0;
}

int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    if (procId == root) {
        // Receive data from other processes
        pthread_t threadId[numProc];
        for (int sourceProc = 0; sourceProc < numProc; sourceProc ++) {
            if (sourceProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->proc = sourceProc;
            threadArg->count = recvcount;
            threadArg->datatype = recvtype;
            threadArg->buf = (void*) ((char *) recvbuf + (recvcount * msizeof(recvtype) * sourceProc));
            pthread_create(&threadId[sourceProc], NULL, rootReceiveData, (void *) threadArg);
        }
        void *destBuf = (void *)((char *) recvbuf + (recvcount * msizeof(recvtype) * root));
        memcpy(destBuf, sendbuf, sendcount * msizeof(sendtype));

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


int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    if (procId == root) {
        // Send to all the processes
        pthread_t threadId[numProc];
        for (int destProc = 0; destProc < numProc; destProc ++) {
            if (destProc == root) continue;
            threadArgs_t *threadArg = (threadArgs_t *) malloc(sizeof(threadArgs_t));
            threadArg->proc = destProc;
            threadArg->count = sendcount;
            threadArg->datatype = sendtype;
            threadArg->buf = (void *) ((char *) sendbuf + (sendcount * msizeof(sendtype) * destProc));
            pthread_create(&threadId[destProc], NULL, rootSendData, (void *) threadArg);
        }
        memcpy(recvbuf, sendbuf, sendcount * msizeof(sendtype));

        for (int i = 0; i < numProc; i ++) {
            if (i == root) continue;
            pthread_join(threadId[i], NULL);
        }
    } else {
        // Receive from root process
        MPI_Status status;
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, MPI_COMM_WORLD, &status);
    }
    return 0;
}


int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    if (MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, 0, comm) < 0) {
        printError("MPI_Allgather", "Error in gather step!");
        return -1;
    };
    if (MPI_Bcast(recvbuf, recvcount * numProc, recvtype, 0, comm) < 0) {
        printError("MPI_Allgather", "Error in broadcast step!");
        return -1;
    };
    return 0;
}

int performOp(MPI_Op op, void* accLoc, void* reduceLoc, MPI_Datatype datatype) {
    switch (op) {
        case MPI_SUM:
            PERFORMOP(+, accLoc, reduceLoc, datatype)
            break;
        case  MPI_MAX:
            PERFORMF(std::max, accLoc, reduceLoc, datatype)
            break;
        case MPI_MIN:
            PERFORMF(std::min, accLoc, reduceLoc, datatype)
            break;
        case MPI_PROD:
            PERFORMOP(*, accLoc, reduceLoc, datatype)
            break;
        case MPI_LAND:
            PERFORMOP(&&, accLoc, reduceLoc, datatype)
            break;
        case MPI_LOR:
            PERFORMOP(||, accLoc, reduceLoc, datatype)
            break;
        default:
            return -1;
    }
    return 0;
} 

int validateDatatype(MPI_Datatype datatype, MPI_Op op) {
    if (datatype == MPI_CHAR) return -1;
    if ((op == MPI_LAND || op == MPI_LOR ) && (datatype == MPI_FLOAT || datatype == MPI_DOUBLE)) {
            return -1;
    } 
    return 0;
}

int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
    if (validateDatatype(datatype, op) < 0) {
        printError("MPI_Reduce", "Unexpected datatype %d for operation %d!", datatype, op);
        return -1;
    }
    if (procId == root) {
        void* gatherbuf = malloc(msizeof(datatype) * count * numProc);
        MPI_Gather(sendbuf, count, datatype, gatherbuf, count, datatype, root, comm);
        memcpy(recvbuf, gatherbuf, msizeof(datatype) * count);
        for (int i=0; i<count; i++) {
            for (int j=1; j<numProc; j++) {
                if (performOp(op, 
                    (void*)((char*)recvbuf + i * msizeof(datatype)),  
                    (void*)((char*)gatherbuf + (j*count + i) * msizeof(datatype)),
                    datatype) < 0) {
                    printError("MPI_Reduce", "Operation %d not implemented!", op);
                    return -1;
                };
            }
        }
    } else {
        MPI_Gather(sendbuf, count, datatype, NULL, count, datatype, root, comm);
    }
    return 0;    
}

// naive implementation
int MPI_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    if (MPI_Reduce(sendbuf, recvbuf, count, datatype, op, 0, comm) < 0) {
        printError("MPI_Allreduce", "Error in reduction step!");
        return -1;
    }
    if (MPI_Bcast(recvbuf, count, datatype, 0, comm) < 0) {
        printError("MPI_Allreduce", "Error in broadcast step!");
        return -1;
    }
    return 0;
}

// tree based implementation, only supported if numProc is a power of 2.
int MPI_treereduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
    if (root != 0) {
        printError("MPI_treereduce", "Tree reduce only supported for root = 0!");
        return -1;
    }
    if (procId == 0) { // root main reducer
        int largestPowerOf2 = (numProc & ~(numProc-1));
        for (int i=1; i<largestPowerOf2; i<<=1) {
            MPI_Status status;
            MPI_Recv(recvbuf, count, datatype, i, 0, comm, &status);
            for (int j=0; j<count; j++) {
                if (performOp(op, 
                    (void*)((char*)sendbuf + j * msizeof(datatype)),  
                    (void*)((char*)recvbuf + j * msizeof(datatype)),
                    datatype) < 0) {
                    printError("MPI_treereduce", "Operation %d not implemented!", op);
                    return -1;
                };
            }
        }
        memcpy(recvbuf, sendbuf, count * msizeof(datatype));
    } else {
        int rank = procId;
        int largestPowerOf2 = (rank & ~(rank-1));
        for (int i=1; i<largestPowerOf2; i<<=1) {
            MPI_Status status;
            MPI_Recv(recvbuf, count, datatype, procId + i, 0, comm, &status);
            for (int j=0; j<count; j++) {
                if (performOp(op, 
                    (void*)((char*)sendbuf + j * msizeof(datatype)),  
                    (void*)((char*)recvbuf + j * msizeof(datatype)),
                    datatype) < 0) {
                    printError("MPI_treereduce", "Operation %d not implemented!", op);
                    return -1;
                };
            }
        }

        MPI_Send(sendbuf, count, datatype, rank - largestPowerOf2, 0, comm);
    }
    return 0;
}

double MPI_Wtime() {
    using namespace std::chrono;
    auto tp = std::chrono::high_resolution_clock::now();
    return (double) tp.time_since_epoch().count() / 1e9;
}