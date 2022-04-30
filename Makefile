# Makefile to compile all the source files of the MPI C++ Library

# Directories
SRCDIR := src
LIBDIR := lib
OBJDIR := objs
EXEDIR := exec

# MPI Object Files
LIBOBJ := $(OBJDIR)/mpi.o $(OBJDIR)/socket.o $(OBJDIR)/rio.o

# Executables
EXECUTABLES := helloworld average wireroute

# Compiler

CXX := g++ -m64 -std=c++11
CXXFLAGS := -pthread -I. -O3 -Wall

CXX_MPI := mpic++ 
CXXFLAGS_MPI := -I. -O3

CXX_APP := $(CXX)
CXXFLAGS_APP := $(CXXFLAGS)

# CXX_APP := $(CXX_MPI)
# CXXFLAGS_APP := $(CXXFLAGS_MPI)

.PHONY : all clean dirs

default : dirs mpirun $(EXECUTABLES)

# Executables
.SECONDEXPANSION:
mpirun : $$(patsubst %,$(OBJDIR)/%.o,$$@) $(LIBOBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

.SECONDEXPANSION:
$(EXECUTABLES) : $$(patsubst %,$(OBJDIR)/%.o,$$@) $(LIBOBJ)
	$(CXX_APP) $(CXXFLAGS_APP) $^ -o $(EXEDIR)/$@

# Library Object Files
$(OBJDIR)/%.o : $(LIBDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

# Application Code Objects
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX_APP) $< $(CXXFLAGS_APP) -c -o $@


dirs :
	mkdir -p $(OBJDIR)/ $(EXEDIR)/

clean:
	rm -rf $(OBJDIR) $(EXEDIR) mpirun WireRoute



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