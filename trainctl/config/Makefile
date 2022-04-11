
CNFS=boards.cnf  canton.cnf  global.cnf  topology.cnf    train.cnf   turnout.cnf

all:.configdone

.configdone: ../config_gen/config $(CNFS)
	cat $(CNFS) | ../config_gen/config 

../config_gen/config:
	(cd ../config_gen && make)
