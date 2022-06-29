/*
 * screen.h
 *
 *  Created on: 28 juin 2022
 *      Author: danielbraun
 */

#ifndef IHM_SCREEN_H_
#define IHM_SCREEN_H_



typedef struct screen {
	int layout;
	const struct screen  *(*handle_trainmode)(train_mode_t);
	const struct screen  *(*handle_button)(uint32_t button_events);
	const struct screen  *(*handle_events)(uint32_t *pevent);
	const struct screen  *button_a_screen;
	const struct screen  *button_b_screen;
} screen_t;

#define IHM_EVT_TRAINMODE		(1<<0)
#define IHM_EVT_BUTTON_A		(1<<1)
#define IHM_EVT_BUTTON_B		(1<<2)

typedef struct {
	const screen_t *screen;
	uint32_t pending_events;
	train_mode_t trainmode;
} ihm_disp_state_t;

extern const screen_t ihm_screen_init;
extern const screen_t ihm_screen_off;
extern const screen_t ihm_screen_train_auto;
extern const screen_t ihm_screen_train_manual;
extern const screen_t ihm_screen_train_off;
extern const screen_t ihm_screen_master;
extern const screen_t ihm_screen_slave;
extern const screen_t ihm_screen_testcan;
extern const screen_t ihm_screen_testcanton;
extern const screen_t ihm_screen_detect2;



int ihm_screen_handle(ihm_disp_state_t *state);

static inline void ihm_screen_set_train_mode(ihm_disp_state_t *state, train_mode_t tm)
{
	if (state->trainmode == tm) return;
	state->trainmode = tm;
	state->pending_events = IHM_EVT_TRAINMODE;
}

#endif /* IHM_SCREEN_H_ */
