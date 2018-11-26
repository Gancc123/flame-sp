# env.mk
# ROOT: the path of 'src'
#	before include this file, you have to define ROOT

ISRC = -I$(ROOT)

DCOMMON = $(ROOT)/common
DMETASTORE = $(ROOT)/metastore
DORM = $(ROOT)/orm
DPROTO = $(ROOT)/proto
DSERVICE = $(ROOT)/service
DUTIL = $(ROOT)/util

CC = gcc
CXX = g++

THREADLIBS	= -lpthread
CXXFLAGS = -std=c++11
DBGFLAGS = -g -std=c++11