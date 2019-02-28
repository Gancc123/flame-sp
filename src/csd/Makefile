ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk
include $(ROOT)/mk/objs.mk
# include $(ROOT)/mk/spdk.mk

OBJ_CSD = csd.o csd_admin.o
OBJ_DEPS = \
$(DSERVICE)/internal_client.o \
$(DSERVICE)/csds_service.o

.PHONY: all clean

all: csd

csd: $(OBJ_CSD) $(OBJ_COMMON) $(OBJ_CHUNKSTORE) $(OBJ_PROTO) $(OBJ_UTIL) $(OBJ_WORK) $(OBJ_DEPS)
	$(CXX) $(DBGFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(GRPC_LDFLAGS) $(AIOLIBS) # $(ISPDK) $(SPDK_CFLAGS) $(SPDK_LIB) $(DPDK_CFLAGS)

%.o: %.cc
	$(CXX) $(DBGFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS) # $(ISPDK)

clean:
	rm -f *.o csd