#  LED module

LED module control layout leds (houses, possibly signals, etc..)

module is driven by LED_Tasklet(), supposed to be called at 1000Hz

(defined in ledtask.c). Due to its high frequency (compared to PWM frequency) tasklet is run in its own 
RTOS task, at higher priority

We do not use harware PWM (since we want to be able to drive many leds), but a bit sequence, which 
allows preprogrammed values (e.g. 100%, 50%, 25%) but also more complex sequences (e.g. simulating old fluorescent tubes lightening)

bit sequeces are defined with a 32 bit based bytecode :

8 bits of cmde | 24 bit sequence

0 r r r r r r r | seq : sequence executed r+1 time; r=0x7F : infinty

1 0 0 0 0 0 0 0 | w    : jump to word w

1 0 0 0 0 0 0 1 | p    : jump to program P

1 0 0 0 0 0 1 0 | n    : loop n time

1 1 1 1 1 1 0 0 | seq  : end-loop

1 1 1 1 1 1 1 s | -    : end prog, with val = s
