# IHM tasklet

IHM behaviour is handled by ihm.c while disp.c manage predefined screen

Current implementation is still limited to a single ssd1306 display, though provision had been made to handle
several displays (targetting 4 displays for UI board)

disp.c has predefined display layout, and ihm only select the layout, and set "register" values that are used by
the layout

Layout are to be extended to include navigation items.

Currently, ihm.c handles both local diagnostic displau (on master board and possibly on slave boards, displaying local state and/or errors)
and real driving UI (but this may change in future)
