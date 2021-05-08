/*
 * itm_debug.h
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#ifndef UTILS_ITM_DEBUG_H_
#define UTILS_ITM_DEBUG_H_


extern uint32_t debug_flags;

#define DBG_TIM  	(1<<0)
#define DBG_MSG		(1<<1)
#define DBG_PID		(1<<2)
#define DBG_INERTIA	(1<<3)
#define DBG_SPDCTL	(1<<4)
#define DBG_INA3221 (1<<5)
#define DBG_PRES	(1<<6)
#define DBG_CONFIG	(1<<7)
#define DBG_LOWCTRL	(1<<8)
#define DBG_TURNOUT	(1<<9)
#define DBG_CTRL	(1<<10)
#define DBG_UI		(1<<11)

#define DBG_ERR		(1<<31)

//void _itm_debug1(const char *msg, int v);
//void _itm_debug2(const char *msg, int v1, int v2);
void _itm_debug3(const char *msg, int v1, int v2, int v3, int n);

static inline void itm_debug1(uint32_t f, const char *msg, int v)
{
	if (f & debug_flags) _itm_debug3(msg, v, 0, 0, 1);
}
static inline void itm_debug2(uint32_t f, const char *msg, int v1, int v2)
{
	if (f & debug_flags) _itm_debug3(msg, v1, v2, 0, 2);
}
static inline void itm_debug3(uint32_t f, const char *msg, int v1, int v2, int v3)
{
	if (f & debug_flags) _itm_debug3(msg, v1, v2, v3, 3);
}

#endif /* UTILS_ITM_DEBUG_H_ */
