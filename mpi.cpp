#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>
#include <errno.h>
#include "socket.h"
#include <sys/socket.h> /* struct sockaddr */
#include <sys/types.h>  /* struct sockaddr */
#include "rio.h"
#include <string.h>

#define LISTEN_PORT_OFFSET 2500
#define MAXBUF 1024 
static int nProc;
static int pId;
static int listenfd = -1;

static inline int getPortId (int procId) {
    return LISTEN_PORT_OFFSET + procId;
}

static inline void getPortString(int procId, char* listenPortStr, int size) {
    snprintf(listenPortStr, size, "%d", getPortId(procId));
}

int MPI_Init(int *argc, char*** argv) {
    nProc = atoi(*argv[0]);
    pId = atoi((*argv)[1]);
    *argc = *argc - 2;
    *argv = *argv + 2;

    char listenPortStr[10];
    getPortString(pId, listenPortStr, 10);
    printf("LISTENPORTSTR %s\n", listenPortStr);
    listenfd = open_listenfd(listenPortStr);
    if (listenfd < 0) {
        printf("Process ID: %d - open_listenfd: returned %d with errno %d\n", pId, listenfd, errno);
    }
    return 0;
}

int MPI_Comm_rank(int* procId) {
    *procId = pId;
    return 0;
}

int MPI_Comm_size(int* numProc) {
    *numProc = nProc;
    return 0;
}

int MPI_Finalize(void) {
    return 0;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest) {
    char destPortStr[10];
    getPortString(dest, destPortStr, 10);
    int destFd = open_clientfd("localhost", destPortStr);
    ssize_t bytes_transacted = rio_writen(destFd, buf, count * datatype);
    if (bytes_transacted < (ssize_t)(count * datatype)) {
        printf("Short write detected!\n");
        return -1;
    }
    close(destFd);
    return 0;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, MPI_Status *status) {
    int client_fd = -1;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = 0;
    clientlen = sizeof(struct sockaddr_storage);
    client_fd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
    if (client_fd < 0) {
        printf("Accept failed!\n");
        return -1;
    }

    rio_t client_rio;
    rio_readinitb(&client_rio, client_fd);
    char msg_buffer[MAXBUF] = {'\0'};
    ssize_t bytes_transacted;
    ssize_t readUntilNow = 0;
    while  ((bytes_transacted = rio_readnb(&client_rio, msg_buffer, MAXBUF)) != 0) {
        if (readUntilNow > (ssize_t)(count * datatype)) break;
        memcpy((char*)buf + readUntilNow, msg_buffer, bytes_transacted);
        readUntilNow += bytes_transacted;
    }
    close(client_fd);
    return 0;
}