ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all simstore clean

all: simstore cs.o

simstore:
	make -C ./simstore

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./simstore clean
	rm -f *.o