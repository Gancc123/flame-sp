ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all example clean

all: timer_work.o example

example:
	make -C ./example

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./example clean
	rm -f *.o