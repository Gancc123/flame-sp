ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all queue clean

all: queue

queue:
	make -C ./queue

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C queue clean
	rm -f *.o