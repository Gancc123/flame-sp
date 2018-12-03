ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: test

test: test.cc 
	$(CXX) $(CXXFLAGS) $^ -o $@ $(ISRC)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o test