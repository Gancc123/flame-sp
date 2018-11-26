ROOT = ../../src

include $(ROOT)/mk/env.mk

all: sqlms ms.o

sqlms: ./sqlms/*.cc ./sqlms/*.h
	make -C ./sqlms

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./sqlms clean
	rm -f *.o