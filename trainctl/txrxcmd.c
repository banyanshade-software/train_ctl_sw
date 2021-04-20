/*
 * txrxcmd.c
 *
 *  Created on: Oct 25, 2020
 *      Author: danielbraun
 */



/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


/* txrxcmd.h : process command and send notification to control software
 * 		for now, control runs on mac, communicating with USB.
 * 		it is expected that the same command processing can be used
 * 		- on BT between main board and tablet / computer
 * 		- on I2C (or other) between main board and slave boards
 */

/*
 * frame mode
 */

#include <stdint.h>
#include <memory.h>

#include "trainctl_iface.h"

#include "txrxcmd.h"

#include "railconfig.h"
//#include "canton.h"
#include "train.h"
//#include "turnout.h"
//#include "traincontrol.h"
#include "auto1.h"
#include "misc.h"
#include "param.h"
#include "statval.h"

//#include "stm32/txframe.h"
//#include "stm32/taskauto.h"
#include "msg/trainmsg.h"

static uint8_t cli_frame_mode=1;  // XXX to be removed


#define FRAME_DELIM '|'
#define FRAME_ESC   '\\'

static int _frm_escape(uint8_t *buf, int len, int maxlen)
{
    int ne = 0;
    for (int i=0; i<len; i++) {
        if ((FRAME_ESC==buf[i]) || (FRAME_DELIM==buf[i])) {
            ne++;
        }
    }
    int nl = ne+len;
    if (nl>maxlen) return -1;
    for (int i=len-1+ne; i>=0; i--) {
        buf[i] = buf[i-ne];
        if ((FRAME_ESC==buf[i]) || (FRAME_DELIM==buf[i])) {
            buf[i-1]=FRAME_ESC;
            ne--;
            i--;
        }
    }
    return nl;
}

static int _frm_escape2(uint8_t *buf,  uint8_t *org, int len, int maxlen)
{
    int ne = 0;
    for (int i=0; i<len; i++) {
    	if (ne>=maxlen) return -1;
        if ((FRAME_ESC==org[i]) || (FRAME_DELIM==org[i])) {
        	buf[ne++] = FRAME_ESC;
        	if (ne>=maxlen) return -1;
        }
        buf[ne++] = org[i];
    }
    return ne;
}
static int frm_escape(uint8_t *buf, int len, int maxlen)
{
	//configASSERT(buf[0]==FRAME_DELIM);
	//configASSERT(buf[len-1]==FRAME_DELIM);
    int l = _frm_escape(buf+1, len-2, maxlen-2);
    if (l<0) return l;
    buf[l+1]=FRAME_DELIM;
    return l+2;
}


#define MAX_DATA_LEN 32

typedef struct {
	uint8_t escape:1;
	uint8_t state:7;
	uint8_t seqnum;
	uint8_t sel;
	uint8_t num;
	uint8_t cmd;
	uint8_t pidx;
	uint8_t param[MAX_DATA_LEN];
	//uint8_t resp[MAX_DATA_LEN];
} frame_state_t;

static frame_state_t frm = {0};

static  uint8_t process_frame_cmd(uint8_t sel, uint8_t num,  uint8_t cmd,
		uint8_t *param, int plen, uint8_t *rbuf, int rbuflen, int *prlen);

static void txframe_send_msg64(msg_64_t *msg);

void usbPollQueues(void)
{
    for (;;) {
        msg_64_t m;
        int rc = mqf_read_to_forward_usb(&m);
        if (rc) break;
        txframe_send_msg64(&m);
    }
}

void txrx_process_char(uint8_t c, uint8_t *respbuf, int *replen)
{
	int buflen = *replen;
	*replen = 0;
	if ((c == FRAME_DELIM) && !frm.escape) {
		if (0 == frm.state) {
			memset(&frm, 0, sizeof(frm));
			frm.state = 1;
			return;
		} else if (5 == frm.state) {
			frm.state = 0;
			frm.escape = 0;
			// process frame
			int i = 0;
			respbuf[i++] = FRAME_DELIM;
			respbuf[i++] = frm.seqnum;
			respbuf[i++] = 'R';
			int plen = 0;
			uint8_t rc = process_frame_cmd(frm.sel, frm.num, frm.cmd, frm.param, frm.pidx, respbuf+i+1, buflen-i-1, &plen);
			respbuf[i++] = rc;
			respbuf[i+plen] = FRAME_DELIM;
			int l = frm_escape(respbuf, i+plen+1, buflen);
			if (l<0) {
				l = 0;
			}
			*replen = l;
			/*if ((0)) {
				static frame_msg_t m;
				m.len = 1+ sprintf(m.frm, "frm resp %d\r\n", *replen);
				txframe_send_debug(&m, 0);
			}*/
			return;
		} else {
			// short frame, ignore
			frm.state = 1;
			frm.escape = 0;
			return;
		}
	}
	if (c==FRAME_ESC && !frm.escape) {
		// state is >0 here
		frm.escape = 1;
		return;
	}

	//  |sSNCvv...|
	switch (frm.state) {
	default:
		frm.escape = 0;
		//configAssert(0);
		break;
	case 1:
		frm.seqnum = c;
		frm.state = 2;
		break;
	case 2:
		frm.sel = c;
		frm.state = 3;
		break;
	case 3:
		frm.num = c;
		frm.state = 4;
		break;
	case 4:
		frm.cmd = c;
		frm.state = 5;
		break;
	case 5:
		if (frm.pidx >= MAX_DATA_LEN) {
			frm.state = 0;
			frm.escape = 0;
			break;
		}
		frm.param[frm.pidx] = c;
		frm.pidx ++;
		break;
	}
        frm.escape = 0;
}




// ---------------------------------------------------------------------------------
static void param_set_pwm(struct param *p, int32_t v) // XXX to be moved away
{
        set_pwm_freq(v);
}

/*
int32_t (*getter)(struct param *);
	void (*setter)(struct param *, int32_t);
*/

static int32_t param_get_numtrains(param_t *p)
{
	return NUM_TRAINS;
}

static int32_t param_get_numcantons(param_t *p)
{
	return NUM_CANTONS;
}

static const param_t glob_params[] = {
		{ "pwmfreq",    &cur_freqhz, 0, NULL, param_set_pwm, sizeof(int), 0, 60000,  50},
		{ "numtrains",   NULL, 0, 	    param_get_numtrains,  NULL, sizeof(uint32_t), 1, 1, 10},
		{ "numcantons",  NULL, 0, 	    param_get_numcantons, NULL, sizeof(uint32_t), 2, 1, 50},
		//{ "test_mode",   &trainctl_test_mode, 0, 	    NULL, NULL, sizeof(uint8_t), 2, 1, 50},

		{ NULL,     NULL,0,    NULL,NULL, 0, 0, 0,   0}
};




static uint8_t process_frame_cmd(uint8_t sel, uint8_t num,  uint8_t cmd, uint8_t *param, int plen, uint8_t *rbuf, int rbuflen, int *prlen)
{
    //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
	int16_t s;
	*prlen = 0;
	int32_t v,d,min,max;
	int rc=-1;

	switch(sel) {
	case 'A':
		switch (cmd) {
		case 'S':
			//rc = turnout_cmd(num, 1);
			break;
		case 's':
			//rc = turnout_cmd(num, -1);
			break;
		case 'W':
			//rc = turnout_test(num);
			break;
		}
		break;
	case 'G':
		switch (cmd) {
		case 'C':
			cli_frame_mode = 0;
			return 0;
			break;
		case 'S':
			// TODO train_stop_all();
			return 0;
			break;
		case 'K':
			// TODO calibrate_bemf();
			return 0;
			break;
		case 'p':
			rc = param_get_value(glob_params, NULL, (char *)param, &v, &d, &min, &max);
			memcpy(rbuf, &v,   sizeof(int32_t)); rbuf += sizeof(int32_t);
			memcpy(rbuf, &d,   sizeof(int32_t)); rbuf += sizeof(int32_t);
			memcpy(rbuf, &min, sizeof(int32_t)); rbuf += sizeof(int32_t);
			memcpy(rbuf, &max, sizeof(int32_t)); rbuf += sizeof(int32_t);
			*prlen = 4*sizeof(int32_t);
			return rc;
		case 'P': {
			if (plen < 4+1) return 1;
			int32_t v;
			memcpy(&v, param, sizeof(int32_t));
			int rc = param_set_value(glob_params, NULL, (char *)(param+sizeof(int32_t)), v);
			return rc;
		}
		default:
			return 3;
		}
		break;
	case 'T':
		//if (num == '0') num=0; // XXX hook for test
		//if (num != 0) return 4; // for now
		switch (cmd) {
		case 'V':
			if (plen !=2) return 1;
			memcpy(&s, param, sizeof(s));
		    train_set_target_speed(num, s);
			return 0;
		case 'A':
			task_auto_start_auto();
			break;
		case 'a':
			task_auto_stop_auto();
			break;
		case 'z':
			rc = 0; // TODO train_reset_pos_estimate(num);
			return rc;
			break;
		case 'p': {
			const train_config_t *tcnf = get_train_cnf(num);
			if (tcnf) {
				rc = param_get_value(train_params, (void *)tcnf, (char *)param, &v, &d, &min, &max);
				memcpy(rbuf, &v,   sizeof(int32_t)); rbuf += sizeof(int32_t);
				memcpy(rbuf, &d,   sizeof(int32_t)); rbuf += sizeof(int32_t);
				memcpy(rbuf, &min, sizeof(int32_t)); rbuf += sizeof(int32_t);
				memcpy(rbuf, &max, sizeof(int32_t)); rbuf += sizeof(int32_t);
				*prlen = 4*sizeof(int32_t);
			} else {
				rc = 3;
			}
			return rc;
		}

		case 'P': {
			if (plen < 4+1) return 1;
			const train_config_t *tcnf = get_train_cnf(num);
			if (tcnf) {
				int32_t v;
				memcpy(&v, param, sizeof(int32_t));
			    rc = param_set_value(train_params, (void *)tcnf, (char *)(param+sizeof(int32_t)), v);
			} else {
				rc = 33;
			}
			return rc;
		}

		default:
			return 33;
		}
		break;
	default:
		rbuf[0]=sel;
		rbuf[1]=num;
		rbuf[2]=cmd;
		*prlen = 3;
		return 5;
		break;
	}
	return 0;
}

static int num_trainctl_notif = 0;

void trainctl_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen)
{
	num_trainctl_notif++;
	if (cli_frame_mode) {
		frame_send_notif(sel, num, cmd, dta, dtalen);
	}
}

void trainctl_notif2(uint8_t sel, uint8_t num, uint8_t cmd, char *msg, int32_t v1, int32_t v2, int32_t v3)
{
	static frame_msg_t m;
	char *p;
	int i=0;
	m.frm[i++] = '|';
	m.frm[i++] = '_';
	m.frm[i++] = 'N';
	m.frm[i++] = sel;
	m.frm[i++] = num;
	m.frm[i++] = cmd;
#if 0
	p = strncpy((char *)m.frm+i, msg, 10);
	m.frm[i+10]='\0';
	i += strlen(p);
	m.frm[i++] = ' ';
	p = itoa(v1, (char*)m.frm+i, 10);
	i += strlen(p);
	m.frm[i++]= '/';
	p = itoa(v2, (char *)m.frm+i, 10);
	i += strlen(p);
#else
	memcpy(m.frm+i, &v1, sizeof(int32_t));
	i += sizeof(int32_t);
	memcpy(m.frm+i, &v2, sizeof(int32_t));
	i += sizeof(int32_t);
	memcpy(m.frm+i, &v3, sizeof(int32_t));
	i += sizeof(int32_t);
 	p = strncpy((char *)m.frm+i, msg, 10);
	m.frm[i+10]='\0';
	i += strlen(p);
#endif
	m.frm[i++]= '\0';
	m.frm[i++] = '|';
	int l = frm_escape(m.frm, i, FRM_MAX_LEN);
	if (l>0) txframe_send_notif(&m, l);
}


void frame_send_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen)
{
	static frame_msg_t m;
	if (dtalen>FRM_MAX_LEN-8) dtalen=FRM_MAX_LEN-8;
	int i=0;
	m.frm[i++] = '|';
	m.frm[i++] = '_';
	m.frm[i++] = 'N';
	m.frm[i++] = sel;
	m.frm[i++] = num;
	m.frm[i++] = cmd;
	memcpy(m.frm+i, dta, dtalen);
	i += dtalen;
	m.frm[i++] = '|';
	int l = frm_escape(m.frm, i, FRM_MAX_LEN);
	if (l>0) txframe_send_notif(&m, l);
}

static void txframe_send_msg64(msg_64_t *msg)
{
    static frame_msg_t m;
    //int dtalen = 8;
    int i=0;
    m.frm[i++] = '|';
    m.frm[i++] = '_';
    m.frm[i++] = '6';
    m.frm[i++] = msg->to;
    m.frm[i++] = msg->from;
    m.frm[i++] = msg->rbytes[0];
    m.frm[i++] = msg->rbytes[1];
    m.frm[i++] = msg->rbytes[2];
    m.frm[i++] = msg->rbytes[3];
    m.frm[i++] = msg->rbytes[4];
    m.frm[i++] = msg->rbytes[5];
    //memcpy(m.frm+i, dta, dtalen);
    //i += dtalen;
    m.frm[i++] = '|';
    int l = frm_escape(m.frm, i, FRM_MAX_LEN);
    if (l>0) txframe_send_notif(&m, l);
}



// buf should be long enough to store a int32_t with escape, so 8 bytes
int frame_gather_stat(int step, uint8_t *buf)
{
	// int32_t stat_val_get(int step);
	int done;
	int32_t v = stat_val_get(step, &done);
	if (done) return 0;

	int l = _frm_escape2(buf, (void *) &v, 4, 8);
	if (l<0) {
		return -1;
	}
	return l;
}

void frame_send_stat(void(*cb)(uint8_t *d, int l), uint32_t tick)
{
    uint8_t buf[8];
    //if ((1)) tick = 0xAA55AA55;
    int l = _frm_escape2(buf, (void *) &tick, 4, 8);
    cb(buf, l);

	int i;
	for (i=0; ; i++) {
		l = frame_gather_stat(i, buf);
		if (l<=0) {
			return;
		}
		cb(buf, l);
	}
}

