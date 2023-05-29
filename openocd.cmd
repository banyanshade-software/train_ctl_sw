#!/bin/sh

/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.openocd.macos64_2.2.200.202210200901/tools/bin/openocd "-f" "stm32f4_mainV4.cfg" "-s" "/Users/danielbraun/devel/train/sw/stm32f4_mainV4" "-s" "/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.debug.openocd_2.0.400.202211031408/resources/openocd/st_scripts" "-c" "gdb_report_data_abort enable" "-c" "gdb_port 3333" "-c" "tcl_port 6666" "-c" "telnet_port 4444"
