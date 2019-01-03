ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk

.PHONY: all clean

all: easdl.o eamou.o poll_layout.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o