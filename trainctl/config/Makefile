
CNFS=boards.cnf  canton.cnf  global.cnf  topology.cnf    train.cnf   turnout.cnf led.cnf globparam.cnf unittest.cnf servo.cnf

all:.configdone

.configdone: ../config_gen/config $(CNFS)
	cat $(CNFS) | ../config_gen/config && touch .configdone

../config_gen/config:
	(cd ../config_gen && make)

clean:
	rm -f conf_*.h conf_*.c .config_done
