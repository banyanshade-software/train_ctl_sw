#!/bin/sh

P=/Users/danielbraun/devel/train/_sw_lt7

openocd -f myocd.cfg \
    "-f" "G491_Slv1.cfg" \
    "-s" "$P/G491_Slv1" \
    "-c" "gdb_report_data_abort enable" "-c" "gdb_port 3333" "-c" "tcl_port 6666" "-c" "telnet_port 4444"


#    "-s" "/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.debug.openocd_2.0.400.202211031408/resources/openocd/st_scripts" 
