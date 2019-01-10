ROOT = ../../src

include $(ROOT)/mk/env.mk

STORE_MODULES = simstore filestore

.PHONY: all $(STORE_MODULES) clean

all: $(STORE_MODULES) cs.o

simstore:
	make -C ./simstore

filestore:
	make -C ./filestore

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./simstore clean
	make -C ./filestore clean
	rm -f *.o