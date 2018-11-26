ROOT = ../../src

include $(ROOT)/mk/env.mk

all: stmt.o my_impl engine.o

my_impl: ./my_impl/*.h ./my_impl/*.cc
	make -C ./my_impl

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./my_impl clean
	rm -f *.o