
all: traceviewer

CCFLAGS += -g -Wall
OBJS=traceviewer.o tracetool.o

traceviewer: $(OBJS)
	$(CXX) -o $@  $^ $(LDFLAGS)


clean: 
	rm -f $(OBJS)
