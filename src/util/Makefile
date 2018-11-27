ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: str_util.o fs_util.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o