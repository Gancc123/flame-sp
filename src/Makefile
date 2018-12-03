ROOT = ../src

include $(ROOT)/mk/env.mk

MODULES = util common orm metastore chunkstore proto service mgr csd cml

.PHONY: all $(MODULES) clean

all: $(MODULES)

util:
	make -C ./util

common:
	make -C ./common

orm:
	make -C ./orm

metastore:
	make -C ./metastore

chunkstore:
	make -C ./chunkstore

proto:
	make -C ./proto

service:
	make -C ./service

mgr:
	make -C ./mgr

csd:
	make -C ./csd

cml:
	make -C ./cml

clean:
	make -C util clean
	make -C common clean
	make -C orm clean
	make -C metastore clean
	make -C chunkstore clean
	make -C proto clean
	make -C service clean
	make -C mgr clean
	make -C csd clean
	make -C cml clean
	rm -f *.o