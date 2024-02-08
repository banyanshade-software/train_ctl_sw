#!/bin/sh

P=/Users/danielbraun/devel/train/_sw_M8detect
C=/opt/openocd


while true; do
	$C/bin/openocd -f myocd.cfg \
    		"-f" "$P/stm32f4_mainV4/stm32f4_mainV4.cfg" \
		"-s" "$P/stm32f4_mainV4" \
    		"-s" "$C/st_scripts" \
		"-c" "gdb_report_data_abort enable"\
		"-c" "gdb_port 3333" \
		"-c" "tcl_port 6666" "-c" "telnet_port 4444" 
	sleep 1
	echo "================================================"
done

#	-d -l ocd.log
