
CXX = g++ -m64 -std=c++11
CXXFLAGS = -I. -O3 -Wall

# Executables
mpirun: mpirun.o mpi.o socket.o
	$(CXX) $(CXXFLAGS) mpirun.o mpi.o socket.o -o mpirun

output: helloworld.o mpi.o socket.o
	$(CXX) $(CXXFLAGS) helloworld.o mpi.o socket.o -o output

# Object Files
mpirun.o: mpirun.cpp
	$(CXX) $(CXXFLAGS) -c mpirun.cpp

mpi.o: mpi.cpp
	$(CXX) $(CXXFLAGS) -c mpi.cpp

socket.o: socket.cpp
	$(CXX) $(CXXFLAGS) -c socket.cpp

helloworld.o: helloworld.cpp
	$(CXX) $(CXXFLAGS) -c helloworld.cpp 

clean: 
	rm -rf *.o output mpirun