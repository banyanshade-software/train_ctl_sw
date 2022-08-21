/*
 * tasklet.h
 *
 *  Created on: Jun 15, 2022
 *      Author: danielbraun
 */

#ifndef MSG_TASKLET_H_
#define MSG_TASKLET_H_

#include "trainmsg.h"


typedef void (*msg_handler_t)(msg_64_t *m);
typedef void (*tick_handler_t)(uint32_t tick, uint32_t dt);

typedef struct {

	void 			(*init)(void);
	int				(*poll_divisor)(uint32_t tick, uint32_t dt);
	void			(*emergency_stop)(void);
	void			(*enter_runmode)(runmode_t);
	tick_handler_t   pre_tick_handler;
	msg_handler_t	 default_msg_handler;
	tick_handler_t 	 default_tick_handler;

	msg_handler_t	(*msg_handler_for)(runmode_t);
	tick_handler_t	(*tick_handler_for)(runmode_t);

	uint8_t			recordmsg:1;

} tasklet_def_t;

typedef struct {
	const tasklet_def_t	*def;
	mqf_t			*queue;
	runmode_t 		runmode;
	uint32_t		last_tick;
	uint8_t			init_done;
} tasklet_t;

void tasklet_run(tasklet_t *tasklet, uint32_t tick);


void tasklet_run_all(tasklet_t *tasklets[]);


#endif /* MSG_TASKLET_H_ */
