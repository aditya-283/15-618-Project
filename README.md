# Implementing MPI as a C++ library

## Team 
Aditya Bhagwat (abhagwa2@andrew.cmu.edu)

Karthik Bhat   (kbhat2@andrew.cmu.edu)

## Summary
In this project, we will implement a subset of Message Passing Interface (MPI) as a C++ library to parallelize programs across multiple cores of a single node. We aim to achieve good performance and scalability and evaluate these on multiple benchmarks. 

## Background 
The subset of MPI that we aim to implement is - 
* Basic operations: MPI_Init, MPI_Finalize, MPI_Comm_size, MPI_Comm_rank
* Point-to-point Communication: We will implement synchronous communication primitives - MPI_Send and MPI_Recv.
* Collective Communication: MPI_Bcast, MPI_Scatter, MPI_Gather, MPI_Reduce

## Challenges
The main challenge of this project is getting a bug-free implementation that implements message passing interface between processes without assuming a shared-memory architecture. This is important because MPI is used in scenarios with multiple physical nodes where sharing memory is not possible at all. All communication will therefore happen via channels (aka pipes) which can be replaced with sockets in the case of a networked cluster. 

Inter-process-communication (IPC) is difficult to get right and incorrect IPC can easily lead to deadlocks and starvation which we will need to prevent. Evaluation will also be challenging because not all bugs make themselves visible on every run of a parallel program - we will require a comprehensive test suite.

The implementation will have challenges at every step - it is not immediately obvious, for example, how all processes will agree on a set of IDs from 0 to N-1 to start with (where N is an arbitrary number of processes that the user decides to spawn).  After correctness, performance and scalability will be our next major concerns - our implementation should have minimal overhead so that a perfectly data-parallel application without blocking synchronization or communication achieves nearly perfect speedup. 

This project will require considerable design - we would like there to be re-use between the different communication primitives. If we design well our initial P2P communication well, our library will support collective communication and reduction at a smaller additional cost. We are decidedly not aiming for completeness here, our stretch goal excludes asynchronous communication. The stretch goal does however include writing efficient versions of broadcast, scatter, gather and reduce instructions exploring all-ring reduce and other efficient communication strategies. 


## Resources
We will run parallel programs using our library on GHC and PSC.

## Goals and Deliverables
The main goal of this project is a functional and correct implementation of MPI that is reasonably performant - applications scale with increasing processor cores.

75% - Basics of process spawning and teardown, synchronous point to point communication
100% - Collective communication - MPI_Bcast. Ensuring correctness and performance scaling on a test program
120% - MPI_Scatter, MPI_Gather, MPI_Reduce

## Schedule
Week 10 - 04/02 - Process spawning, identification and teardown.

Week 11 - 04/09 - Setup interprocess communication using channels to implement P2P communication primitives.

Project checkpoint  - 04/11 

Week 12 - 04/16 - Implement collective commmunication primitives.

Week 13 - 04/23 - Benchmarking and correctness evaluation

Week 14 - 04/29 - Writing documentation, final project report and preparing a demo.

Project Report      - 04/29

Presentation        - 05/05

## References
[1] https://www.open-mpi.org/
[2] https://hpc-tutorials.llnl.gov/mpi/