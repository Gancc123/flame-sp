ROOT = ../../../src

include $(ROOT)/mk/env.mk

all: str_util.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o