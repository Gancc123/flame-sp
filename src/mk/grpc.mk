IGRPC += -I/usr/local/include/google/protobuf -I/usr/local/include/grpc++ -I/usr/local/include/grpc
GRPC_LIB = -lgrpc++ -lgrpc -lprotobuf
GRPC_LDFLAGS += -L/usr/local/lib -L/usr/lib -lpthread $(GRPC_LIB)\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl