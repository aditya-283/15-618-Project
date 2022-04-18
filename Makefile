
CXX = g++ -m64 -std=c++11
CXXFLAGS = -pthread -I. -O3 -Wall

# Run all executables
all: mpirun output wireroute 
.PHONY : all

# Executables
mpirun : mpirun.o mpi.o socket.o rio.o
	$(CXX) $(CXXFLAGS) mpirun.o mpi.o socket.o rio.o -o mpirun

output : helloworld.o mpi.o socket.o rio.o
	$(CXX) $(CXXFLAGS) helloworld.o mpi.o socket.o rio.o -o output

average : average.o mpi.o socket.o rio.o
	$(CXX) $(CXXFLAGS) average.o mpi.o socket.o rio.o -o average

serialAverage : serialAverage.o 
	$(CXX) $(CXXFLAGS) serialAverage.o  -o serialAverage

wireroute : wireroute.o mpi.o socket.o rio.o
	$(CXX) $(CXXFLAGS) wireroute.o mpi.o socket.o rio.o -o wireroute

# Object Files
mpirun.o : mpirun.cpp 
	$(CXX) $(CXXFLAGS) -c mpirun.cpp 

mpi.o : mpi.cpp
	$(CXX) $(CXXFLAGS) -c mpi.cpp 

rio.o : rio.cpp
	$(CXX) $(CXXFLAGS) -c rio.cpp 

socket.o : socket.cpp
	$(CXX) $(CXXFLAGS) -c socket.cpp

helloworld.o : examples/helloworld.cpp
	$(CXX) $(CXXFLAGS) -c examples/helloworld.cpp 

average.o : examples/average.cpp
	$(CXX) $(CXXFLAGS) -c examples/average.cpp 

serialAverage.o : examples/serialAverage.cpp
	$(CXX) $(CXXFLAGS) -c examples/serialAverage.cpp 

wireroute.o: examples/wireroute.cpp
	$(CXX) $(CXXFLAGS) -c examples/wireroute.cpp 


clean : 
	rm -rf *.o output average mpirun serialAverage wireroute examples/outputs/** examples/costs/**
.PHONY : clean