ROOT = ../../../src

include $(ROOT)/mk/env.mk

all: mutex.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o