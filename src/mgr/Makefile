ROOT = ../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/grpc.mk
include $(ROOT)/mk/mysql.mk

all: mgr

mgr: mgr.cc $(DPROTO)/flame.grpc.pb.o $(DPROTO)/flame.pb.o $(DSERVICE)/flame_service.o context.o log.o
	$(CXX) $(CXXFLAGS) $(IGRPC) $^ -o $@ $(ISRC) $(GRPC_LDFLAGS)

context.o: $(DCOMMON)/context.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

log.o: $(DCOMMON)/log.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

str_util.o: $(DUTIL)/str_util.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o mgr