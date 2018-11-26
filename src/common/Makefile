ROOT = ../../src

include $(ROOT)/mk/env.mk

all: thread context.o log.o config.o cmdline.o cli_test

thread: ./thread/*.h ./thread/*.cc
	make -C ./thread

cli_test: cli_test.cc cmdline.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(ISRC)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./thread clean
	rm -f *.o