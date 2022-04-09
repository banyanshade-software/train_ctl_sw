

all: config

CFLAGS += -g

config: config.o system.o utility.o main.o

config.c: config.peg
	packcc -d -l config.peg

clean:
	rm -f *.o *.log

test: c1.log c2.log c3.log c4.log c5.log c6.log c7.log c8.log

%.log: %.cnf config
	./config < $< > $@
