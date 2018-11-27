ROOT = ../src

include $(ROOT)/mk/env.mk

MODULES = util common orm metastore proto service mgr

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

proto:
	make -C ./proto

service:
	make -C ./service

mgr:
	make -C ./mgr

clean:
	make -C util clean
	make -C common clean
	make -C orm clean
	make -C metastore clean
	make -C proto clean
	make -C service clean
	make -C mgr clean
	rm -f *.o