ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk

.PHONY: all clean

all: calculator.o poll_layout.o easdl.o eamou.o 

%.o: %.cc
	$(CXX) $(DBGFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o