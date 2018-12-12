ROOT = ../../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/objs.mk

.PHONY: all clean

all: lockless

lockless: lockless.cc $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(ISRC) $(THREADLIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o lockless