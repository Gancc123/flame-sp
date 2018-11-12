ROOT = ../../../src

I_SRC = -I $(ROOT)
I_GTESTI = -I $(ROOT)/googletest/googletest/include
I_GTESTD = -I $(ROOT)/googletest/googletest
I_ALL = $(I_SRC) $(I_INCLUDE) $(I_GTESTI) $(I_GTESTD)

CC = gcc
CXX = g++
CXXFLAGS = -std=c++11 -lpthread -pthread
DBG_FLAGS = -g -std=c++11

MYSQLFLAGS = -lmysqlclient -lmysqlcppconn
MYSQLLIB = -L/usr/local/mysql/lib

DCOM = $(ROOT)/common
DUTIL = $(ROOT)/util
DORM = $(ROOT)/orm

FGTEST = $(ROOT)/googletest/googletest/src/gtest-all.cc

all: driver_exp.exe engine_exp.exe stmt_exp.exe stmt_test.exe

driver_exp.exe: driver_exp.cc stmt.o str_util.o my_impl.o
	$(CXX) $(CXXFLAGS) $(MYSQLFLAGS) $^ -o $@ $(I_ALL) $(MYSQLLIB)

engine_exp.exe: engine_exp.cc stmt.o str_util.o my_impl.o engine.o mutex.o
	$(CXX) $(CXXFLAGS) $(MYSQLFLAGS) $^ -o $@ $(I_ALL) $(MYSQLLIB)

mutex.o: $(ROOT)/common/thread/mutex.cc
	$(CXX) $(CXXFLAGS) $^ -c $(I_ALL)

engine.o: $(DORM)/engine.cc
	$(CXX) $(CXXFLAGS) $^ -c $(I_ALL)

my_impl.o: $(DORM)/my_impl/my_impl.cc
	$(CXX) $(CXXFLAGS) $(MYSQLFLAGS) $^ -c -o my_impl.o $(I_ALL) $(MYSQLLIB)

stmt_exp.exe: stmt_exp.cc stmt.o str_util.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(I_ALL)

stmt_test.exe: stmt_test.cc stmt.o str_util.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(I_ALL)

stmt.o: $(DORM)/stmt.cc
	$(CXX) $(CXXFLAGS) $^ -c $(I_ALL)

str_util.o: $(DUTIL)/str_util.cc
	$(CXX) $(CXXFLAGS) $^ -c $(I_ALL)

gtest-all.o: $(FGTEST)
	$(CXX) $(CXXFLAGS) $^ -c -o gtest-all.o $(I_ALL)

clean:
	rm -rf *.exe
	rm -rf *.o