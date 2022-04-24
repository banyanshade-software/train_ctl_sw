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
#include "cmsis_os.h"
#include "main.h"
#endif
#include "trainctl_config.h"

#include "utils/itm_debug.h"
#include "oam/oam_error.h"

void flash_led(void);

#define INCLUDE_CALIB 0

#define _UNUSED_ __attribute__((unused))


#define SIGNOF(_v) (((_v)<0) ? -1 : 1)
#define SIGNOF0(_v) (((_v)<0) ? -1 : (((_v)>0) ? 1 : 0))

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
	itm_debug1(DBG_CONFIG, msg, code);
	trainctl_error('G', code, msg);
	return NULL;
}

static inline void *runtime_error(int code, const char *msg)
{
	itm_debug1(DBG_CONFIG, msg, code);
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

//void Error_Handler(void);


#ifndef TRAIN_SIMU
uint32_t HAL_GetTick(void);

static inline uint32_t GetCurrentMicro(void)
{
  uint32_t m0 = HAL_GetTick();
  uint32_t u0 = SysTick->LOAD - SysTick->VAL;
  uint32_t m1 = HAL_GetTick();
  uint32_t u1 = SysTick->LOAD - SysTick->VAL;

  if (!SysTick->LOAD) return m0*1000;

  if (m1 > m0) {
    return ( m1 * 1000 + (u1 * 1000) / SysTick->LOAD);
  } else {
    return ( m0 * 1000 + (u0 * 1000) / SysTick->LOAD);
  }
}
#endif

void long_isr(uint32_t dur);


//
// https://electronics.stackexchange.com/questions/76098/high-resolution-system-timer-in-stm32
//
uint64_t GetCycleCount64(void);
void startCycleCounter(void);


#if 0
#define BEGIN_ISR \
	volatile uint32_t isr_tm0 = GetCurrentMicro();

#define END_ISR do { \
	uint32_t isr_tm1 = GetCurrentMicro(); \
	if (isr_tm1 - isr_tm0 > 1000) { long_isr(isr_tm1-isr_tm0); } \
} while (0)
#else

#define BEGIN_ISR \
	volatile uint64_t isr_tm0 = GetCycleCount64();

#define END_ISR do { \
		uint64_t isr_tm1 = GetCycleCount64(); \
	if (isr_tm1 - isr_tm0 > 48000) { long_isr(isr_tm1-isr_tm0); } \
} while (0)

/* 96MHz : 1ms = 96000 cycles
 * long = 500us 48000 cycles */
#endif

// ---------------------------------------------
extern int tsktick_freqhz;	// freq (derived from pwm freq) where most tasklet (ctrl) are called


// ---------------------------------------------
// stat vals

/*
typedef enum {
    stat_per_train,
    stat_per_blk,
    stat_per_subblk
} stat_type_t;

// stat domain
#define STAT_CTRL                   (0<<1)
#define STAT_SPDCTL                 (0<<2)
#define STAT_PRES                   (0<<3)

extern uint8_t stat_generation;
extern uint32_t stat_domain;
void stat_register(void *ptr, int siz, int sign, stat_type_t t, int idx);

#define STAT_VAL(_domain, _value, _stat_type, _n) do {                  \
    static uint8_t _sg = 0;                                             \
    if (_sg != stat_generation) {                                       \
        if (_domain & stat_domain) {                                    \
            int s = sizeof(typeof(_value));                             \
            int is_signed = ((typeof(_value))(-1) < (typeof(_value))(0));  \
            stat_register(&(_value),s, is_signed, (_stat_type), (_n)) ; \
        }                                                               \
        _sg = stat_generation;                                          \
    }                                                                   \
} while(0)
*/


#endif /* TRAINCTL_MISC_H_ */
