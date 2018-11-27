ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: flame

flame: flame.cc $(DCOMMON)/cmdline.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(ISRC)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o