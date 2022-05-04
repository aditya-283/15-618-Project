#ifndef MPI_H
#define MPI_H

typedef struct MPI_Status_ {
    int count;
    int cancelled;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;

enum MPI_Comm {MPI_COMM_WORLD};

typedef enum MPI_Datatype_ {
    MPI_CHAR = 0,
    MPI_INT = 1,
    MPI_FLOAT = 2,
    MPI_LONG = 3,
    MPI_DOUBLE = 4
} MPI_Datatype;

typedef enum MPI_Op_ {
    MPI_MAX,
    MPI_MIN,
    MPI_SUM,
    MPI_PROD,     
    MPI_LAND,          
    MPI_LOR
} MPI_Op;

int MPI_Init(int *argc, char*** argv);

int MPI_Comm_rank(MPI_Comm comm, int* rank);

int MPI_Comm_size(MPI_Comm comm, int* size);

int MPI_Finalize(void);

int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm);

int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);

int MPI_treereduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);

int MPI_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);

int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);

double MPI_Wtime();

#endif
