# env.mk
# ROOT: the path of 'src'
#	before include this file, you have to define ROOT

ISRC = -I$(ROOT)

DCHUNKSTORE = $(ROOT)/chunkstore
DCOMMON = $(ROOT)/common
DMETASTORE = $(ROOT)/metastore
DORM = $(ROOT)/orm
DPROTO = $(ROOT)/proto
DSERVICE = $(ROOT)/service
DUTIL = $(ROOT)/util
DWORK = $(ROOT)/work
DCLUSTER = $(ROOT)/cluster
DLAYOUT = $(ROOT)/layout
DSPOLICY = $(ROOT)/spolicy

CC = gcc
CXX = g++

AIOLIBS = -laio
THREADLIBS	= -lpthread
THREADLIB = -pthread
CXXFLAGS = -std=c++11
DBGFLAGS = -g -std=c++11