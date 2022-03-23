# Implementing MPI as a C++ library

## Team 
Aditya Bhagwat (abhagwa2@andrew.cmu.edu)

Karthik Bhat   (kbhat2@andrew.cmu.edu)

## Summary
In this project, we will implement a subset of Message Passing Interface (MPI) as a C++ library to parallelize programs across multiple cores of a single node. We aim to achieve good performance and scalability and evaluate these on multiple benchmarks. 

## Background 
The subset of MPI that we aim to implement is - 
* Basic operations: `MPI_Init`, `MPI_Finalize`, `MPI_Comm_size`, `MPI_Comm_rank`
* Point-to-point Synchronous Communication: `MPI_Send` and `MPI_Recv`.
* Collective Communication: `MPI_Bcast`, `MPI_Scatter`, `MPI_Gather`, `MPI_Reduce`

## Challenges
The main challenge of this project is getting a bug-free implementation that implements message passing interface between processes without assuming a shared-memory architecture. This is important because MPI is used in scenarios with multiple physical nodes where sharing memory is not possible at all. All communication will therefore happen via channels (aka pipes) which can be replaced with sockets in the case of a networked cluster. 

The first challenge we will face is in getting processes to agree on a set of IDs from 0 to N-1 (where N is an arbitrary number of processes that the user decides to spawn). A general implementation of messsage passing will require a three-phase transaction which creates many possibilities for deadlocks - we will need logically separate input and output queues between processes to avoid them. 

After correctness, performance and scalability will be our next major concerns - our implementation should have minimal overhead so that a perfectly data-parallel application without blocking synchronization or communication achieves nearly perfect speedup. We will have to manage input buffer space efficiently and dealing with congestion (many processes sending to one) will pose additional challenges for scalability.

This project will require considerable design - we would like there to be re-use between the different communication primitives. We are not aiming for completeness here and our stretch goal excludes asynchronous communication. However, we aim to write efficient versions of broadcast, scatter, gather and reduce instructions exploring all-ring reduce and other efficient communication strategies. 


## Resources
We will run parallel programs using our library on GHC and PSC.

## Goals and Deliverables
The main goal of this project is a functional and correct implementation of MPI that is reasonably performant - applications scale with increasing processor cores.

Targets:
* 75%  - Basics of process spawning and teardown, synchronous point to point communication
* 100% - Collective communication - `MPI_Bcast`. Ensuring correctness and performance scaling on a test program
* 125% - Efficient implementations of remaining collective communication primitives - `MPI_Scatter`, `MPI_Gather`, `MPI_Reduce`

## Schedule
* Week of 03/27 - Process spawning, identification and teardown.
* Week of 04/03 - Setup interprocess communication using channels to implement P2P communication primitives.
* Week of 04/10 - Implement collective commmunication primitives.

**Project checkpoint  - 04/11**  
* Week of 04/17 - Benchmarking and correctness evaluation
* Week of 04/24 - Writing documentation, final project report and preparing a demo.  

**Project Report      - 04/29**

**Presentation        - 05/05**

## References
[1] https://www.open-mpi.org/

[2] https://hpc-tutorials.llnl.gov/mpi/
