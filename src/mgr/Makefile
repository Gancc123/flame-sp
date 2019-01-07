ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk
include $(ROOT)/mk/objs.mk

OBJ_MGR_CSDM = \
$(ROOT)/mgr/csdm/csd_mgmt.o \
$(ROOT)/mgr/chkm/chk_mgmt.o

OBJ_MGR = mgr.o mgr_server.o 

.PHONY: all csdm chkm clean

all: csdm chkm mgr

csdm:
	make -C ./csdm
	make -C ./chkm

mgr: $(OBJ_MGR) $(OBJ_MGR_CSDM) $(OBJ_COMMON) $(OBJ_METASTORE) $(OBJ_PROTO) $(OBJ_SERVICE) $(OBJ_ORM) $(OBJ_UTIL) $(OBJ_WORK) $(OBJ_CLUSTER) $(OBJ_LAYOUT)
	$(CXX) $(CXXFLAGS) $(MYSQL_CFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(MYSQL_LIB) $(GRPC_LDFLAGS)  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS)

clean:
	make -C ./csdm clean
	make -C ./chkm clean
	rm -f *.o mgr