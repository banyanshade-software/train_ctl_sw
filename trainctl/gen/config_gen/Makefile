

PACKCC=../../../tools/packcc/build/clang/debug/bin/packcc

all: config 

CFLAGS += -g -Wall -Werror

SRCS=config.c system.c utility.c main.c gen.c nodeutil.c

OBJS = $(SRCS:%.c=%.o)

config: $(OBJS)

config.c: config.peg
	$(PACKCC)  -l config.peg
	
clean:
	rm -f *.o *.log .depend conf_*

test: tests/c1.log tests/c2.log tests/c3.log tests/c4.log tests/c5.log tests/c6.log tests/c7.log tests/c8.log

tests/%.log: tests/%.cnf config
	./config < $< > $@


depend: .depend

.depend: $(SRCS)
	rm -f "$@"
	$(CC) $(CFLAGS) -MM $^ > "$@"

include .depend
