include ../env.mk

str_ut: str_ut.cc $(DUTIL)/str_util.cc gtest-all.o
	$(CXX) $(CXXFLAGS) $(DBG_FLAGS) $^ -o $@ $(I_ALL)

clean:
	rm -f *_ut
	rm -f *.o
