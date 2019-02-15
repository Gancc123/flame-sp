ROOT = ../../src

include $(ROOT)/mk/env.mk

STORE_MODULES = simstore filestore #nvmestore

.PHONY: all $(STORE_MODULES) clean

all: $(STORE_MODULES) cs.o

simstore:
	make -C ./simstore

filestore:
	make -C ./filestore

# nvmestore:
	# make -C ./nvmestore
	
%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./simstore clean
	make -C ./filestore clean
	# make -C ./nvmestore clean
	rm -f *.o