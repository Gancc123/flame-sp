ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: spolicy.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o