#
# include this file after env.mk
#

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
$(DPROTO)/internal.grpc.pb.o

# /service
OBJ_SERVICE = \
$(DSERVICE)/flame_client.o \
$(DSERVICE)/flame_service.o \
$(DSERVICE)/internal_service.o