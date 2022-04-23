/*
 * framing.c
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */


#include "misc.h"
#include "framing.h"



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

int txrx_frm_escape2(uint8_t *buf,  uint8_t *org, int len, int maxlen)
{
	return _frm_escape2(buf, org, len, maxlen);
}
