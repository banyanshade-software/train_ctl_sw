

all: config

CFLAGS += -g -Wall -Werror

SRCS=config.c system.c utility.c main.c gen.c

OBJS = $(SRCS:%.c=%.o)

config: $(OBJS)

config.c: config.peg
	#packcc -d -l config.peg
	packcc  -l config.peg
	

clean:
	rm -f *.o *.log .depend

test: c1.log c2.log c3.log c4.log c5.log c6.log c7.log c8.log

%.log: %.cnf config
	./config < $< > $@


depend: .depend

.depend: $(SRCS)
	rm -f "$@"
	$(CC) $(CFLAGS) -MM $^ > "$@"

include .depend
