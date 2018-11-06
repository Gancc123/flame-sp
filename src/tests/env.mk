ROOT = ../../../src

I_SRC = -I $(ROOT)
I_GTESTI = -I $(ROOT)/googletest/googletest/include
I_GTESTD = -I $(ROOT)/googletest/googletest
I_ALL = $(I_SRC) $(I_INCLUDE) $(I_GTESTI) $(I_GTESTD)

CC = gcc
CXX = g++
CXXFLAGS = -std=c++11 -lpthread -pthread
DBG_FLAGS = -g -std=c++11

DCOM = $(ROOT)/common
DUTIL = $(ROOT)/util

FGTEST = $(ROOT)/googletest/googletest/src/gtest-all.cc

gtest-all.o: $(FGTEST)
	$(CXX) $(CXXFLAGS) $^ -c -o gtest-all.o $(I_ALL)