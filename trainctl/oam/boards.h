/*
 * boards.h
 *
 *  Created on: Apr 3, 2022
 *      Author: danielbraun
 */

#ifndef OAM_BOARDS_H_
#define OAM_BOARDS_H_

//int boardIdToBoardNum(uint32_t uuid);



/// - returns 1 if board is master
int oam_isMaster(void);


/// returns local board number
///
/// for master (oam_isMaster()==1) : always returns 0
///
/// for slaves : returns -1 before  board number allocation is finished
int oam_localBoardNum(void);


/// boards master resolultion
///
/// - board determines if it can be master (is board TRN_BOARD_MAIN* ?)
/// - board checks flash to see if it is configured as master
/// - if slave: normal slave behaviour (see bellow)
///    if no answer : become master ???
/// - if master : broadcast CMD_OAM_MASTER with board uniqid
/// master goes in runmode_master, where it handles oam cmd
/// - if master receive CMD_OAM_MASTER : do ???
///

/// board number allocations
/// slave boards starts in oam_slave mode
/// slave regularly sends CMD_OAM_SLAVE with board unique id to OAM(0)
/// if board is known in config, master reply with CMD_OAM_BNUM, v32=unique id, subc=boad number
/// slv boards goes to runmode_off and sends CMD_OAM_SLV_OK, from=OAM(bnum) to=OAM(0)  v1=version
/// if board is unknown : master reply with CMD_OAM_BNUM, v32=unique id, subc=0xFF
/// master propagate params to slave



/// for slave, store the assigned board number
/// @param bnum assigned board number (received in subc of CMD_OAM_BNUM)
void oam_localBoardNum_set(uint8_t bnum);

/// on master, return the board number of a slave, given its unique id
/// returns -1 if board unique id is unknown
int oam_boardForUuid(uint32_t uniquid);

#endif /* OAM_BOARDS_H_ */
