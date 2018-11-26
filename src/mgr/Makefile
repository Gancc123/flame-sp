ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk

OBJ_COMMON = $(DCOMMON)/context.o $(DCOMMON)/log.o $(DCOMMON)/config.o
OBJ_PROTO = $(DPROTO)/flame.pb.o $(DPROTO)/flame.grpc.pb.o \
			$(DPROTO)/internal.pb.o $(DPROTO)/internal.grpc.pb.o
OBJ_SERVICE = $(DSERVICE)/flame_service.o $(DSERVICE)/internal_service.o 

all: mgr

mgr: mgr.cc $(OBJ_COMMON) $(OBJ_PROTO) $(OBJ_SERVICE) mgr_service.o
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(GRPC_LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o mgr