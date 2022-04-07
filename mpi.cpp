#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>
#include <errno.h>
#include "socket.h"
#include <sys/socket.h> /* struct sockaddr */
#include <sys/types.h>  /* struct sockaddr */
#include "rio.h"

#define LISTEN_PORT_OFFSET 2000
#define MAXBUF 1024 
static int nProc;
static int pId;
static int listenfd = -1;

static inline int getPortId (int procId) {
    return LISTEN_PORT_OFFSET + procId;
}

int MPI_Init(int *argc, char*** argv) {
    nProc = atoi(*argv[0]);
    pId = atoi((*argv)[1]);
    *argc = *argc - 2;
    *argv = *argv + 2;

    int listenPort = getPortId(pId);
    char listenPortStr[10];
    snprintf(listenPortStr, sizeof(listenPortStr), "%d", listenPort);
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

int MPI_Send(const void *buf, int count, MPI_Datatype MPI_Datatype, int dest) {
    // Connect to dest process

    // Send from Buffer

    // Close connection
}

int MPI_Recv(void *buf, int count, MPI_Datatype MPI_Datatype, int source, MPI_Status *status) {
    int client_fd = -1;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = 0;
    clientlen = sizeof(struct sockaddr_storage);
    client_fd = accept(getPortId(pId), (struct sockaddr*)&clientaddr, &clientlen);
    if (client_fd < 0) {
        printf("Accept failed!\n");
    }

    rio_t client_rio;
    rio_readinitb(&client_rio, client_fd);
    char msg_buffer[MAXBUF] = {'\0'};
    int bytes_transacted;
    while  ((bytes_transacted = ))
    return 0;

    // Recieve

    // Close Connection
}