/*
 * framing.h
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */

#ifndef UTILS_FRAMING_H_
#define UTILS_FRAMING_H_

/*
 2 kinds of frames :
    - fixed msg64 frame :
        0x7E < 8 bytes >
    - variable length frames (currently only stm32->mac ) :
        '|' <1byte type><escaped sequence> '|
        escape char is '\'
        var.len frame types :
            'S' stats
            'V' oscillo
            'M' recorded msg
 */


#define FRAME_DELIM '|'
#define FRAME_ESC   '\\'
#define FRAME_M64   0x7E

/*
 
 */
int txrx_frm_escape2(uint8_t *buf,  uint8_t *org, int len, int maxlen);


void frame_send_oscillo(void(*cb)(const uint8_t *d, int l));
void frame_send_stat(void(*cb)(const uint8_t *d, int l), uint32_t tick);
void frame_send_recordmsg(void(*cb)(const uint8_t *d, int l));


#endif /* UTILS_FRAMING_H_ */
