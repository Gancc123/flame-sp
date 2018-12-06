ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk
include $(ROOT)/mk/objs.mk

OBJ_MGR = mgr.o mgr_server.o

.PHONY: all clean

all: mgr

mgr: $(OBJ_MGR) $(OBJ_COMMON) $(OBJ_METASTORE) $(OBJ_PROTO) $(OBJ_SERVICE) $(OBJ_ORM) $(OBJ_UTIL)
	$(CXX) $(CXXFLAGS) $(MYSQL_CFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(MYSQL_LIB) $(GRPC_LDFLAGS)  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS)

clean:
	rm -f *.o mgr