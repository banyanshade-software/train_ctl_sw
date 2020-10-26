#ifndef TRAINCTL_H
#define TRAINCTL_H

#include "trainctl_config.h"
#if TRAINCTL_NEW == 0

#include "param.h"

// low level
// dir = -1 / 0 / 1
void trainlow_set_speed(int dir, int speed_pourcent);
void trainlow_volt(int v);

// inertia

typedef struct {
	int minpwm;
	int maxpwm;
	int acc;
	int dec;
	int volt;
} train_config_t;

typedef struct {
	train_config_t *config;
	int speed_cur;   // signed
	int speed_target; // signed
} train_t;

extern train_config_t trainconf0;
extern train_t train0;

void train_reset(void);
void train_targetspeed(int sp);
void train_set_volt(int v);
// called by timer
void train_update(void);
//void trainlow_volt(int v);

extern const param_t trc_params[];

#endif
#endif

