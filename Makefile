
CXX = g++ -m64 -std=c++11
CXXFLAGS = -I. -O3 -Wall

mpirun: mpirun.o mpi.o 
	$(CXX) $(CXXFLAGS) mpirun.o mpi.o -o mpirun

mpirun.o: mpirun.cpp mpi.cpp mpi.h
	$(CXX) $(CXXFLAGS) -c mpirun.cpp

mpi.o: mpi.cpp
	$(CXX) $(CXXFLAGS) -c mpi.cpp

output: helloworld.o mpi.o
	$(CXX) $(CXXFLAGS) helloworld.o mpi.o -o output

helloworld.o: helloworld.cpp mpirun
	$(CXX) $(CXXFLAGS) -c helloworld.cpp 

clean: 
	rm -rf *.o output mpirun