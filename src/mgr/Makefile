ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk
include $(ROOT)/mk/objs.mk

OBJ_MGR = \
$(ROOT)/mgr/mgr.o \
$(ROOT)/mgr/mgr_server.o \
$(ROOT)/mgr/csdm/csd_mgmt.o \
$(ROOT)/mgr/chkm/chk_mgmt.o	\
$(ROOT)/mgr/volm/vol_mgmt.o

.PHONY: all csdm chkm volm clean

all: csdm chkm volm mgr

csdm:
	make -C ./csdm

chkm:
	make -C ./chkm

volm:
	make -C ./volm

mgr: $(OBJ_MGR) $(OBJ_MGR_CSDM) $(OBJ_COMMON) $(OBJ_METASTORE) $(OBJ_PROTO) $(OBJ_SERVICE) $(OBJ_ORM) $(OBJ_UTIL) $(OBJ_WORK) $(OBJ_CLUSTER) $(OBJ_LAYOUT) $(OBJ_SPOLICY)
	$(CXX) $(DBGFLAGS)  $(IGRPC) $^ -o $@ $(ISRC) $(MYSQL_LIB) $(GRPC_LDFLAGS) $(MYSQL_CFLAGS)

%.o: %.cc
	$(CXX) $(DBGFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS) 

clean:
	make -C ./csdm clean
	make -C ./chkm clean
	make -C ./volm clean
	rm -f *.o mgr