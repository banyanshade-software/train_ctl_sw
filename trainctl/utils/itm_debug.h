/*
 * itm_debug.h
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#ifndef UTILS_ITM_DEBUG_H_
#define UTILS_ITM_DEBUG_H_


extern uint32_t debug_flags;

#define DBG_TIM  	(1UL<<0)
#define DBG_MSG		(1UL<<1)
#define DBG_PID		(1UL<<2)
#define DBG_INERTIA	(1UL<<3)
#define DBG_SPDCTL	(1UL<<4)
#define DBG_INA3221 (1UL<<5)
#define DBG_PRES	(1UL<<6)
#define DBG_CONFIG	(1UL<<7)
#define DBG_LOWCTRL	(1UL<<8)
#define DBG_TURNOUT	(1UL<<9)
#define DBG_CTRL	(1UL<<10)
#define DBG_UI		(1UL<<11)
#define DBG_ADC 	(1UL<<12)
#define DBG_POSE	(1UL<<13)
#define DBG_POSEC	(1UL<<14)
//#define DBG_CTRLHI	(1UL<<15)
#define DBG_TRKPLN	(1UL<<15)
#define DBG_USB		(1UL<<16)
#define DBG_LED     (1UL<<17)
#define DBG_AUTO    (1UL<<18)
#define DBG_DETECT  (1UL<<19)
#define DBG_CAN     (1UL<<20)
#define DBG_OAM     (1UL<<21)
#define DBG_ERR		(1UL<<31)

//void _itm_debug1(const char *msg, int v);
//void _itm_debug2(const char *msg, int v1, int v2);
void _itm_debug3(int err, const char *msg, int32_t v1, int32_t v2, int32_t v3, int n);

#if 1
static inline void itm_debug1(uint32_t f, const char *msg, int32_t v)
{
	if (f & debug_flags) _itm_debug3(f & DBG_ERR, msg, v, 0, 0, 1);
}
static inline void itm_debug2(uint32_t f, const char *msg, int32_t v1, int32_t v2)
{
	if (f & debug_flags) _itm_debug3(f & DBG_ERR, msg, v1, v2, 0, 2);
}
static inline void itm_debug3(uint32_t f, const char *msg, int32_t v1, int32_t v2, int32_t v3)
{
	if (f & debug_flags) _itm_debug3(f & DBG_ERR, msg, v1, v2, v3, 3);
}
#else

#define itm_debug1(_flags, _msg, _v1)           do { _itm_debug3((_flags) & DBG_ERR, _msg, _v1,   0,   0, 1); } while(0)
#define itm_debug2(_flags, _msg, _v1, _v2)      do { _itm_debug3((_flags) & DBG_ERR, _msg, _v1, _v2,   0, 2); } while(0)
#define itm_debug3(_flags, _msg, _v1, _v2, _v3) do { _itm_debug3((_flags) & DBG_ERR, _msg, _v1, _v2, _v3, 3); } while(0)

#endif

void itm_write(const char *str, int len);

#ifdef TRAIN_SIMU
char* itoa ( int32_t  value,  char str[],  int radix);
#endif

#endif /* UTILS_ITM_DEBUG_H_ */
