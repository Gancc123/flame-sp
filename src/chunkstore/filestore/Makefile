ROOT = ../../../src

include $(ROOT)/mk/env.mk

.PHONY: all clean

all: filestore.o filechunk.o chunkutil.o filechunkmap.o object.o filestoreconf.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o