ROOT = ../../../src

include $(ROOT)/mk/env.mk

all: sqlms.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o