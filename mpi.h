#ifndef MPI_H
#define MPI_H

typedef struct MPI_Status_ {
    int count;
    int cancelled;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;


typedef enum MPI_Datatype_ {
    MPI_CHAR = sizeof(char),
    MPI_INT = sizeof(int),
    MPI_FLOAT = sizeof(float),
    MPI_LONG = sizeof(long),
    MPI_DOUBLE = sizeof(double)
} MPI_Datatype;


int MPI_Init(int *argc, char*** argv);

int MPI_Comm_rank(int* procId);

int MPI_Comm_size(int* numProc);

int MPI_Finalize(void);

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, MPI_Status *status);

int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root);

#endif
