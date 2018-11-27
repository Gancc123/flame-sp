ROOT = ../../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/mysql.mk

all: my_impl.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(MYSQL_CFLAGS) $^ -c $(ISRC) $(MYSQL_LIB)

clean:
	rm -f *.o