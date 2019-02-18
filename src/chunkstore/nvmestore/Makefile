ROOT = ../../../src
SPDK_ROOT = $(ROOT)/spdk

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/spdk.mk

include $(SPDK_ROOT)/mk/spdk.common.mk
include $(SPDK_ROOT)/mk/spdk.app.mk
include $(SPDK_ROOT)/mk/spdk.modules.mk

#OBJ_NVMESTORE = nvmestore.o nvmechunk.o nvmeconf.o nvmechunkmap.o chunkblobmap.o iochannels.o

.PHONY: all clean

all: nvmestore.o nvmechunk.o nvmeconf.o nvmechunkmap.o chunkblobmap.o iochannels.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC) $(ISPDK)

clean:
	rm -f *.o