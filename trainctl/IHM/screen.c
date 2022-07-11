/*
 * screen.c
 *
 *  Created on: 28 juin 2022
 *      Author: danielbraun
 */



#include <stdint.h>

#include "misc.h"
#include "disp.h"
#include "../ctrl/ctrl.h"
#include "screen.h"

// dummy screen to use stack pop
const screen_t screen_return = { 0 };

const screen_t ihm_screen_init = {
		.layout = LAYOUT_DEFAULT,
};

const screen_t ihm_screen_off = {
		.layout = LAYOUT_OFF,
};

const screen_t *_handle_normal_trainmode(train_mode_t tm);



const screen_t ihm_screen_master = {
		.layout = LAYOUT_MASTER,
		.button_a_screen = &screen_return,
};

const screen_t ihm_screen_train_auto = {
		.layout = LAYOUT_AUTO,
		.handle_trainmode = _handle_normal_trainmode,
		.button_a_screen = &ihm_screen_master,
};


const screen_t ihm_screen_train_manual = {
		.layout = LAYOUT_AUTO,
		.handle_trainmode = _handle_normal_trainmode,
		.button_a_screen = &ihm_screen_master,
};


const screen_t ihm_screen_train_off = {
		.layout = LAYOUT_NO_TRAIN,
		.handle_trainmode = _handle_normal_trainmode,
		.button_a_screen = &ihm_screen_master,
};


const screen_t ihm_screen_slave = {
		.layout = LAYOUT_SLAVE,
};

const screen_t ihm_screen_testcan = {
		.layout = LAYOUT_TESTCAN,
};

const screen_t ihm_screen_testcanton = {
		.layout = LAYOUT_TESTCANTON,
};

const screen_t ihm_screen_detect2 = {
		.layout = LAYOUT_DETECT2,
};

const screen_t *_handle_normal_trainmode(train_mode_t tm)
{
	switch (tm) {
	case train_notrunning:
		return &ihm_screen_train_off;
	case train_manual:
	case train_fullmanual:
		return &ihm_screen_train_manual;
	case train_auto:
		return &ihm_screen_train_auto;
	default:
		return NULL;
	}
}


// ----------------------------------
static void screen_push(ihm_disp_state_t *state)
{
	for (int i=SCREEN_STACK-1; i>0; i--) {
		state->stack[i]=state->stack[i-1];
	}
	state->stack[0] = state->screen;
}

static const screen_t *screen_pop(ihm_disp_state_t *state)
{
	const screen_t *r = NULL;
	r = state->stack[0];
	if (!r) return r;
	for (int i=0; i<SCREEN_STACK-2; i++) {
		state->stack[i] = state->stack[i+1];
	}
	/*
	 * note : last item is kept and dup when popping
	 *  A                A (instead of nil)
	 *  B	=== pop ==>  A
	 *  C                B
	 *  we're ok with that (at least for now
	 *  (enable line below to set last element of stack to nil )
	 */
	if ((0)) state->stack[SCREEN_STACK-1] = NULL;
	return r;

}
// ----------------------------------

// events that should be cleared when we change screen
// typic. user buttons, since their meaning is screen dependant

#define TRANSIANT_EVENT (IHM_EVT_BUTTON_A|IHM_EVT_BUTTON_B)

static int screen_change(ihm_disp_state_t *state, const screen_t *ns)
{
	if (ns == state->screen) return 0;
	if (ns == &screen_return) {
		ns = screen_pop(state);
		if (!ns) return 0;
	}
	if ((1)) {
		screen_push(state);
	}
	state->screen = ns;
	state->pending_events &= ~TRANSIANT_EVENT;
	return 1;
}

int ihm_screen_handle(ihm_disp_state_t *state)
{
	const screen_t *ns = NULL;
	if (!state->pending_events) return 0;
	int rc = 0;
	if (state->pending_events & IHM_EVT_TRAINMODE) {
		if (state->screen->handle_trainmode) {
			state->pending_events &= ~IHM_EVT_TRAINMODE;
			ns = state->screen->handle_trainmode(state->trainmode);
			rc |= screen_change(state, ns);
		}
	}
	if (state->pending_events & IHM_EVT_BUTTON_A) {
		if (state->screen->button_a_screen) {
			state->pending_events &= ~IHM_EVT_BUTTON_A;
			ns = state->screen->button_a_screen;
			rc |= screen_change(state, ns);
		}
	}
	if (state->pending_events & IHM_EVT_BUTTON_B) {
		if (state->screen->button_b_screen) {
			state->pending_events &= ~IHM_EVT_BUTTON_B;
			ns = state->screen->button_b_screen;
			rc |= screen_change(state, ns);
		}
	}
	if (state->pending_events & (IHM_EVT_BUTTON_A|IHM_EVT_BUTTON_B)) {
		if (state->screen->handle_button) {
			ns = state->screen->handle_button(state->pending_events & (IHM_EVT_BUTTON_A|IHM_EVT_BUTTON_B));
			state->pending_events &= ~IHM_EVT_BUTTON_A|IHM_EVT_BUTTON_B;
			rc |= screen_change(state, ns);
		}
	}
	if (state->pending_events && state->screen->handle_events) {
		ns = state->screen->handle_events(&state->pending_events);
		rc |= screen_change(state, ns);
	}
	return rc;
}
