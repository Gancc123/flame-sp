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
$(DCOMMON)/thread/mutex.o

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
$(DSERVICE)/csds_service.o

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
$(DCHUNKSTORE)/cs.o