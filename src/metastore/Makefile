ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all sqlms clean

all: sqlms ms.o

sqlms:
	make -C ./sqlms

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./sqlms clean
	rm -f *.o