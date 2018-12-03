ROOT = ../../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: simstore.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o