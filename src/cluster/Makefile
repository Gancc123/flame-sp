ROOT = ../../src

include $(ROOT)/mk/env.mk

.PHONY: all clt_my clean

all: clt_my

clt_my:
	make -C ./clt_my

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	make -C ./clt_my clean
	rm -f *.o