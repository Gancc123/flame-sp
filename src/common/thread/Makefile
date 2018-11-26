ROOT = ../../../src

include $(ROOT)/mk/env.mk

all: mutex.o io_priority.o signal.o thread.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC) $(THREADLIBS)

clean:
	rm -f *.o