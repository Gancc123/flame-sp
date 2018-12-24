ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk
include $(ROOT)/mk/objs.mk

OBJ_MGR_CSDM = $(ROOT)/mgr/csdm/csd_mgmt.o

OBJ_MGR = mgr.o mgr_server.o

.PHONY: all csdm clean

all: csdm mgr

csdm:
	make -C ./csdm

mgr: $(OBJ_MGR) $(OBJ_MGR_CSDM) $(OBJ_COMMON) $(OBJ_METASTORE) $(OBJ_PROTO) $(OBJ_SERVICE) $(OBJ_ORM) $(OBJ_UTIL)
	$(CXX) $(CXXFLAGS) $(MYSQL_CFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(MYSQL_LIB) $(GRPC_LDFLAGS)  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS)

clean:
	make -C ./csdm clean
	rm -f *.o mgr