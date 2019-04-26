ROOT = ../../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: chk_mgmt.o

%.o: %.cc
	$(CXX) $(DBGFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o