#  MSG module

MSG offers centralised message passing, based on lock-free circulared buffer

A centralised (one per board) adds some propagation delay but

- allow us to use lock-free circular buffer (single reader and single writer)

- provides routing flexibility (a tasklet may handles several addresses)

- allows easy forwarding to USB and/or to CAN


Message are fixed size 64 bits, and thus can dirrectly be sent on CAN bus
