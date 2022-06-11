#  Topology module

Topology modules is responsible for

- providing to other modules a view of the track topology - wich segment is connected to wich segment
(topology.c)

- storing occupency of each segment, and reservation of turnouts
(occupency.c)

It does not run as a tasklet, it simply provides functions API. It is *not* itended to be thread safe, since it is actually only used by (*at least currenlty*) by ctrl module (main ctrl RTOS task)
