ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk

OBJ_COMMON = $(DCOMMON)/context.o $(DCOMMON)/log.o $(DCOMMON)/config.o \
			$(DCOMMON)/cmdline.o $(DCOMMON)/thread/mutex.o
OBJ_ORM = $(DORM)/stmt.o $(DORM)/my_impl/my_impl.o $(DORM)/engine.o
OBJ_METASTORE = $(DMETASTORE)/sqlms/sqlms.o $(DMETASTORE)/ms.o
OBJ_PROTO = $(DPROTO)/flame.pb.o $(DPROTO)/flame.grpc.pb.o \
			$(DPROTO)/internal.pb.o $(DPROTO)/internal.grpc.pb.o
OBJ_SERVICE = $(DSERVICE)/flame_service.o $(DSERVICE)/internal_service.o
OBJ_UTIL = $(DUTIL)/str_util.o
OBJ_MGR = mgr.o mgr_server.o

.PHONY: all clean

all: mgr

mgr: $(OBJ_MGR) $(OBJ_COMMON) $(OBJ_METASTORE) $(OBJ_PROTO) $(OBJ_SERVICE) $(OBJ_ORM) $(OBJ_UTIL)
	$(CXX) $(CXXFLAGS) $(MYSQL_CFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(MYSQL_LIB) $(GRPC_LDFLAGS)  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -c $(ISRC) $(GRPC_LDFLAGS)

clean:
	rm -f *.o mgr