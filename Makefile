# Makefile to compile all the source files of the MPI C++ Library

# Directories
SRCDIR := src
LIBDIR := lib
OBJDIR := objs
EXEDIR := exe

# MPI Object Files
LIBOBJ := $(OBJDIR)/mpi.o $(OBJDIR)/socket.o $(OBJDIR)/rio.o

# Executables
EXECUTABLES := mpirun helloworld average serialAverage wireroute

# Compiler
CXX := g++ -m64 -std=c++11
CXXFLAGS := -pthread -I. -O3 -Wall

.PHONY : all clean dirs

default : dirs mpirun $(EXECUTABLES)

# Executables
.SECONDEXPANSION:
$(EXECUTABLES) : $$(patsubst %,$(OBJDIR)/%.o,$$@) $(LIBOBJ)
	$(CXX) $(CXXFLAGS) $^ -o $(EXEDIR)/$@

# Library Object Files
$(OBJDIR)/%.o : $(LIBDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

# Application Code Objects
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@


dirs :
	mkdir -p $(OBJDIR)/ $(EXEDIR)/

clean:
	rm -rf $(OBJDIR) $(EXEDIR) mpirun WireRoute/costs WireRoute/outputs



# Makefile Help:
#
# Make Format:
# target: prerequisite
# 	recipe
#
# Make arguments Help:
# $@ - Represents Target 
# $< - First prerequisite
# $? - All prerequisite that are newer
# $^ - All the prerequisites
#
# Assignments
# := - Static Assignment
#  = - Dynamic (Recursive) Assignment