

PACKCC=../../../tools/packcc/build/clang/debug/bin/packcc

all: genmsg

CFLAGS += -g -Wall -Werror

SRCS=genmsg.c main.c system.c utility.c gen.c

OBJS = $(SRCS:%.c=%.o)

genmsg: $(OBJS)

genmsg.c: genmsg.peg
	$(PACKCC)  -l genmsg.peg

clean:
	rm -f *.o *.log tests/*.log .depend conf_*


# ---------------------
depend: .depend

.depend: $(SRCS)
	rm -f "$@"
	$(CC) $(CFLAGS) -MM $^ > "$@"

# ---------------------
test: tests/m1.log tests/m2.log tests/m3.log tests/m4.log tests/m5.log tests/m6.log tests/m7.log

tests/%.log: tests/%.def genmsg
	./genmsg h - < $< > $@



include .depend
