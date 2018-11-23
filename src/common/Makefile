ROOT = ../../src

include $(ROOT)/mk/env.mk

all: thread context.o log.o config.o

thread: ./thread/*
	make -C ./thread

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./thread clean
	rm -f *.o