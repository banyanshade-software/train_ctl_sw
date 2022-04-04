/*
 * cantest.c
 *
 *  Created on: 2 avr. 2022
 *      Author: danielbraun
 */






#include <stdint.h>
#include <memory.h>

#include "../misc.h"
#include "../msg/trainmsg.h"


#include "oam.h"
#include "boards.h"


#ifndef BOARD_HAS_CAN
#error BOARD_HAS_CAN not defined, remove this file from build
#endif


static runmode_t run_mode = 0;

static  int master = 0;

static void exit_can_test(void)
{
	msg_64_t m;
	m.from = MA_BROADCAST;
	m.to = MA_BROADCAST;
	m.cmd = CMD_SETRUN_MODE;
	m.v1u =  runmode_normal;
	mqf_write_from_nowhere(&m);
}

void OAM_Tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	static int first = 1;
	static int respok = 0;
	if ((first)) {
		uint32_t myid = getDeviceId();
		int myboard = boardIdToBoardNum(myid);
		if (0==myboard) {
			master = 1;
		}
		first = 0;
		msg_64_t m;
		m.from = MA_BROADCAST;
		m.to = MA_BROADCAST;
		m.cmd = CMD_SETRUN_MODE;
		m.v1u =  runmode_testcan; // runmode_normal; runmode_detect2; runmode_off

		mqf_write_from_nowhere(&m); // from_nowher, otherwise it wont be sent to self
	}


	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_oam(&m);
		if (rc) break;

		switch (m.cmd) {
		case CMD_SETRUN_MODE:
			if (m.v1u != run_mode) {
				run_mode = (runmode_t) m.v1u;
				if (run_mode == runmode_testcan) {
					respok = 0;
				}
			}
			break;
		case CMD_CANTEST:
			if ((0)) { // XXX test relat
				static int cnt=0;
				cnt++;
				if (0==(cnt%4)) {
					static int a = 0;
					msg_64_t m = {0};
					m.to = MA_TURNOUT(1, 0);
					m.from = MA_OAM(1);
					m.cmd = a ? CMD_TURNOUT_A : CMD_TURNOUT_B;
					a = a ? 0 : 1;
					mqf_write_from_oam(&m);
				}
			}
			if (run_mode != runmode_testcan) break;
			if (master) break;
			m.cmd = CMD_CANTEST_RESP;
			m.v2u = m.v1u*2;
			mqf_write_from_oam(&m);
			break;

		case CMD_CANTEST_RESP:
			// also handled by IHM
			if (run_mode == runmode_testcan) {
				respok++;
				if (respok > 20) {
					respok = 0;
					exit_can_test();
				}
			}
			break;

		default:
			break;
		}
	}
	switch (run_mode) {
	case runmode_testcan: break;
	default:
		return;
	}

	if (!master) return;

	// send ping
	static uint32_t lasttick = 0;

	if (tick>lasttick+1000) {
		lasttick = tick;
		static uint16_t cnt = 0;
		cnt++;
		if (cnt > 999) {
			respok = 0;
			exit_can_test();
			return;
		}

		msg_64_t m = {0};
		m.from = MA_BROADCAST;
		m.to = MA_BROADCAST;
		m.cmd = CMD_CANTEST;
		m.v1u = cnt;

		mqf_write_from_oam(&m);
	}
}

uint32_t getDeviceId(void)
{
	/* Read MCU Id, 32-bit access */
	uint32_t id0 = HAL_GetUIDw0();
	uint32_t id1 = HAL_GetUIDw1();
	uint32_t id2 = HAL_GetUIDw2();

	// stm32 gets a uniq 96 bits id, but this is too large for a msg64_t
	// so we need to reduce it to 32 bits (or at least 64)

	uint32_t xid = id0 ^ id1 ^ id2;
	// XXX TODO use crc32 instead
	return xid;
}
