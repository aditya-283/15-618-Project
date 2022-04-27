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
    MPI_CHAR = sizeof(char),
    MPI_INT = sizeof(int),
    MPI_FLOAT = sizeof(float),
    MPI_LONG = sizeof(long),
    MPI_DOUBLE = sizeof(double)
} MPI_Datatype;


int MPI_Init(int *argc, char*** argv);

int MPI_Comm_rank(MPI_Comm comm, int* rank);

int MPI_Comm_size(MPI_Comm comm, int* size);

int MPI_Finalize(void);

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm);

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);

#endif
