#ifndef MPI_H
#define MPI_H

int MPI_Init(int *argc, char*** argv);

int MPI_Comm_rank(int* procId);

int MPI_Comm_size(int* numProc);

int MPI_Finalize(void);

#endif
