
CXX = g++ -m64 -std=c++11
CXXFLAGS = -pthread -I. -O3 -Wall

# Run all executables
all: mpirun output
.PHONY : all

# Executables
mpirun : mpirun.o mpi.o socket.o rio.o
	$(CXX) $(CXXFLAGS) mpirun.o mpi.o socket.o rio.o -o mpirun

output : average.o mpi.o socket.o rio.o
	$(CXX) $(CXXFLAGS) average.o mpi.o socket.o rio.o -o output

# Object Files
mpirun.o : mpirun.cpp 
	$(CXX) $(CXXFLAGS) -c mpirun.cpp 

mpi.o : mpi.cpp
	$(CXX) $(CXXFLAGS) -c mpi.cpp 

rio.o : rio.cpp
	$(CXX) $(CXXFLAGS) -c rio.cpp 

socket.o : socket.cpp
	$(CXX) $(CXXFLAGS) -c socket.cpp

average.o : average.cpp
	$(CXX) $(CXXFLAGS) -c average.cpp 


clean : 
	rm -rf *.o output mpirun
.PHONY : clean