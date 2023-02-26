
all: traceviewer

CXXFLAGS += -g -Wall -std=c++11
OBJS=traceviewer.o tracetool.o

traceviewer: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@  $^ $(LDFLAGS)


clean: 
	rm -f $(OBJS)
