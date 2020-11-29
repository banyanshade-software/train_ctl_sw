/*
 * misc.h
 *
 *  Created on: Sep 14, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#ifndef TRAINCTL_MISC_H_
#define TRAINCTL_MISC_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#endif

void flash_led(void);

#define INCLUDE_CALIB 0


#define SIGNOF(_v) (((_v)<0) ? -1 : 1)
static inline int signof0(int v)
{
    if (v>0) return 1;
    if (v<0) return -1;
    return 0;
}

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

/* ================================================================= */

#define ERR_UNKNOWN         -1
#define ERR_BAD_PARAM       -2
#define ERR_BAD_PARAM_TIM   -3
#define ERR_BAD_PARAM_MPOW  -4
#define ERR_BAD_PARAM_VPOL  -5
#define ERR_BAD_STATE		-9

#define ERR_SETUP_KO		-10

#define ERR_DMA             -12


#define ERR_CANTON_USED     -100
#define ERR_CANTON_NONE     -101

#define ERR_AUTO1_BADSTATE	-200

#define ERR_STRANGE			-4242

int trainctl_error(char l, int code, const char *msg);

static inline int canton_error(int code, const char *msg)
{
	return trainctl_error('C', code, msg);
}
static inline int canton_error_rc(int rc, int code, const char *msg)
{
	trainctl_error('C', code, msg);
	return rc;
}
static inline int pid_error(int code, const char *msg)
{
	return trainctl_error('P', code, msg);
}
static inline int train_error(int code, const char *msg)
{
	return trainctl_error('T', code, msg);
}
static inline int turnout_error(int code, const char *msg)
{
	return trainctl_error('A', code, msg);
}

static inline void *config_error(int code, const char *msg)
{
	trainctl_error('G', code, msg);
	return NULL;
}

static inline void *runtime_error(int code, const char *msg)
{
	trainctl_error('G', code, msg);
	return NULL;
}

/* ================================================================= */

//void frame_send_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen);

void trainctl_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen);
void trainctl_notif2(uint8_t sel, uint8_t num, uint8_t cmd, char *msg, int32_t v1, int32_t v2, int32_t v3);

static inline void canton_notif(uint8_t canton_idx, uint8_t cmd, uint8_t *dta, int dtalen)
{
	trainctl_notif('C', canton_idx, cmd, dta, dtalen);
}
static inline void global_notif(uint8_t cmd, uint8_t *dta, int dtalen)
{
	trainctl_notif('G', 0, cmd, dta, dtalen);
}

static inline void train_notif(uint8_t train_idx, uint8_t cmd, uint8_t *dta, int dtalen)
{
	trainctl_notif('T', train_idx, cmd, dta, dtalen);
}

static inline void debug_info(uint32_t sel, uint32_t num, char *msg, int v1, int v2, int v3)
{
	trainctl_notif2(sel, num, 'D', msg, v1, v2, v3);
}
/* ================================================================= */



#endif /* TRAINCTL_MISC_H_ */
