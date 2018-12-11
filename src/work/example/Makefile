ROOT = ../../../src

include $(ROOT)/mk/env.mk
include $(ROOT)/mk/objs.mk

.PHONY: all clean

all: timer_exp.o timer_exp

timer_exp: timer_exp.o $(ROOT)/work/timer_work.o $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(ISRC) $(THREADLIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $^ -c $(ISRC)

clean:
	rm -f *.o