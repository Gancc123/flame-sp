#
# include this file after env.mk
#

# /util
OBJ_UTIL = \
$(DUTIL)/str_util.o \
$(DUTIL)/fs_util.o

# /common
OBJ_COMMON = \
$(DCOMMON)/context.o \
$(DCOMMON)/log.o \
$(DCOMMON)/config.o \
$(DCOMMON)/cmdline.o \
$(DCOMMON)/convert.o \
$(DCOMMON)/thread/mutex.o \
$(DCOMMON)/thread/thread.o \
$(DCOMMON)/thread/signal.o \
$(DCOMMON)/thread/io_priority.o

# /proto
OBJ_PROTO = \
$(DPROTO)/flame.pb.o \
$(DPROTO)/flame.grpc.pb.o \
$(DPROTO)/internal.pb.o \
$(DPROTO)/internal.grpc.pb.o \
$(DPROTO)/csds.pb.o \
$(DPROTO)/csds.grpc.pb.o

# /service
OBJ_SERVICE = \
$(DSERVICE)/flame_service.o \
$(DSERVICE)/flame_client.o \
$(DSERVICE)/internal_service.o \
$(DSERVICE)/internal_client.o \
$(DSERVICE)/csds_service.o \
$(DSERVICE)/csds_client.o

# /orm
OBJ_ORM = \
$(DORM)/stmt.o \
$(DORM)/my_impl/my_impl.o \
$(DORM)/engine.o

# /metastore
OBJ_METASTORE = \
$(DMETASTORE)/sqlms/sqlms.o \
$(DMETASTORE)/ms.o

# /chunkstore
OBJ_CHUNKSTORE = \
$(DCHUNKSTORE)/simstore/simstore.o \
$(DCHUNKSTORE)/filestore/filestore.o \
$(DCHUNKSTORE)/filestore/filechunk.o \
$(DCHUNKSTORE)/filestore/filechunkmap.o \
$(DCHUNKSTORE)/filestore/filestoreconf.o \
$(DCHUNKSTORE)/filestore/chunkutil.o \
$(DCHUNKSTORE)/filestore/object.o \
$(DCHUNKSTORE)/cs.o
# $(DCHUNKSTORE)/nvmestore/chunkblobmap.o \ 
# $(DCHUNKSTORE)/nvmestore/iochannels.o \ 
# $(DCHUNKSTORE)/nvmestore/nvmechunkmap.o \ 
# $(DCHUNKSTORE)/nvmestore/nvmechunk.o \ 
# $(DCHUNKSTORE)/nvmestore/nvmeconf.o \ 
# $(DCHUNKSTORE)/nvmestore/nvmestore.o \ 


# /work
OBJ_WORK = \
$(DWORK)/timer_work.o

# /cluster
OBJ_CLUSTER = \
$(DCLUSTER)/clt_my/my_mgmt.o

# /layout
OBJ_LAYOUT = \
$(DLAYOUT)/calculator.o \
$(DLAYOUT)/poll_layout.o

# /spolicy
OBJ_SPOLICY = \
$(DSPOLICY)/spolicy.o
