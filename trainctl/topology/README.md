#  Topology module

Topology modules is responsible for

- providing to other modules a view of the track topology - wich segment is connected to wich segment
(topology.c) and stores turnouts positions

- storing occupency of each segment, and reservation of turnouts
(occupency.c)

It does not run as a tasklet, it simply provides functions API. 

### thread safeness


