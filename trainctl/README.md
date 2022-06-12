#  trainctl lib

this is main part of the project

code is organized with several ''tasklet''  : functions, implementing a FSM, that shall be call regularly, and
communicate by message only, through a messaging  tasklet.
(This is closed to ITU Z-100 SDL model)

## Features

- automatic train (given origin-destination). _not yet implemented_

- manual ''safe'' train driving : through UI board, each train can be driven manually, but system
ensure safety and stop train before collision or end of track. _implemented but for short train only_

- CTC like visualisation. _impletend, but HTML generation is not performed on target, but on PC host_

- PID controlled speed, using BEMF as speed feedback value ; this is intended to give fine control at low speed. _implemented but BEMF is very noisy, so we only use PI. A predictive control might behave better_

- position estimation on canton (based on BEMF), giving the possibility to define logical subblocks (electrically part of canton, but topologically different). _implemented, relatively reiiable if we define 

- current based presence detection. _implemented but I2C/ina3221 tends to crash_

- turnout control

- control of LED in diorama

- other...

## Global architecture

The system is targeted to be multiboard, one board being master (storing configuration, running trackplan, ctrl, and spdctrl, i.e. tasklet running on a per train basis), and controlling the other slave boards, communicating through **CAN bus**


![global architecture](img/garch.001.png "global architecture")
