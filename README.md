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


## [UPDATE] MILESTONE 04/11/2022

We have completed the following - process spawning, identification and teardown, and synchronous point to point communication using the primitives `MPI_Send` and `MPI_Recv`. We have used the same API for all subroutines as MPI, implying we had to implement initialization and finalization functions like `MPI_Init` and `MPI_Finalize` and create an executable `mpirun` that takes a command line argumennt and an executable and spawns identical parallel processes as desired. The passed executable can take arbitrary command line arguments. To have a similar interface, we have also implemented a useful subset of data types of type `MPI_Datatype` which gives us to flexible mechanism to marhsall and interpret communicated data. The above is fully tested on simple message-passing example code, presented in `helloworld.cpp` where odd numbered processes send a message to even numbered processes. We have stress-tested the above for various failure modes and can confirm that our implementation is correct and deadlock-free.
To test - clone the project, run `make` and then invoke the example program using the command `mpirun -n 8 -e ./helloworld` to see message-passing in action. 

In summary, we have been able to deliver what we promised for the checkpoint. In fact, we're ahead of what we anticipated since we've also completed the first version of our code for `MPI_Bcast` and `MPI_Gather`. These remain to be tested rigorously for correctness. After that is done, our next goals, broadly, will be detailed benchmarking on GHC and PSC. The data we collect will help us understand the bottlenecks in our implementation and target better performance. If time permits, we will implement more sophisticated collective communication algorithms.

Following is a schedule for the coming few weeks. 

| Week | Task | Assignee |
| :-: | :-: | :-: |
|  Week of 04/10 | MPI_Bcast and MPI_Gather correctness evaluation| Karthik|
|  Week of 04/10 | Benchmarking code,  comparison with MPI and performance analysis | Aditya|
|  Week of 04/17 | MPI_Reduce (tree-based) | Karthik|
|  Week of 04/17 | MPI_Scatter, MPI_Reduce (ring-reduce) | Aditya|
|  Week of 04/24 | Project demo code, code-clean up, pending bug-fixes | Karthik|
|  Week of 04/24 | Project report with performance analysis  | Aditya|

Our major concerns are about getting comparable performance for parallel programs written using our library to that achieved using MPI. We are confident of achieving our 100% milestone by this week - of correct and performant implementations of `MPI_Bcast` and `MPI_Gather`. We are also bullish on achieving the 125% milestone of writing performant implementations of collective communication and comparing communication topologies (tree-based versus ring-based) for `MPI_Reduce`.

For the poster session, we will demonstrate the working of our library on PSC with 64 or 128 cores. We will choose a computation-heavy parallelizable problem of the right size, such that both sequential and parallel versions finish in the duration and the speed-up with the parallel version is clearly seen.