/*
 * ihm_messages.c
 *
 *  Created on: Feb 12, 2024
 *      Author: danielbraun
 */

#include "misc.h"
#include "ihm_messages.h"

#ifdef BOARD_HAS_IHM

#define _RESERVED(_id) NULL

#ifdef TRAIN_SIMU

#define _M(_id, _short, _long) _long  "\n"
const char *ihmmsglist[] = {

#else
#define _M(_id, _short, _long) _short
const char *ihmmsglist[] = {

#endif
#include "ihm_msg.inc"

};

#endif // BOARD_HAS_IHM
