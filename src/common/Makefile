ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all thread clean

all: thread context.o log.o config.o cmdline.o

thread:
	make -C ./thread

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./thread clean
	rm -f *.o