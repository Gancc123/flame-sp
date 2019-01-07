ROOT = ../src

include $(ROOT)/mk/env.mk

MODULES = util common work orm metastore chunkstore proto service cluster layout spolicy mgr csd cml csm

.PHONY: all $(MODULES) clean

all: $(MODULES)

util:
	make -C ./util

common:
	make -C ./common

work:
	make -C ./work

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

csm:
	make -C ./csm

cluster:
	make -C ./cluster

layout:
	make -C ./layout

spolicy:
	make -C ./spolicy

clean:
	make -C ./util clean
	make -C ./common clean
	make -C ./work clean
	make -C ./orm clean
	make -C ./metastore clean
	make -C ./chunkstore clean
	make -C ./proto clean
	make -C ./service clean
	make -C ./layout clean
	make -C ./mgr clean
	make -C ./csd clean
	make -C ./cml clean
	make -C ./csm clean
	make -C ./cluster clean
	make -C ./spolicy clean
	rm -f *.o