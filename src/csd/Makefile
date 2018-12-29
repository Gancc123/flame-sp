ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk
include $(ROOT)/mk/objs.mk

OBJ_CSD = csd.o csd_admin.o
OBJ_DEPS = \
$(DSERVICE)/internal_client.o \
$(DSERVICE)/csds_service.o

.PHONY: all clean

all: csd

csd: $(OBJ_CSD) $(OBJ_COMMON) $(OBJ_CHUNKSTORE) $(OBJ_PROTO) $(OBJ_UTIL) $(OBJ_WORK) $(OBJ_DEPS)
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(GRPC_LDFLAGS)  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS)

clean:
	rm -f *.o mgr