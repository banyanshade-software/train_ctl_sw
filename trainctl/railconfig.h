/*
 * railconfig.h
 *
 *  Created on: Oct 8, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/* railconfig.h : this is the main storage for configuration and runtime info
 *                related to train, blocks, turnout, etc..
 *                We basically maitains arrays for all trains, all blocks, etc..
 * 				  Configuration upload/download is planned here, but not implemented
 */

#ifndef RAILCONFIG_H_
#define RAILCONFIG_H_

#include "trainctl_iface.h"

#ifdef TRAIN_SIMU
#include "train_simu.h"
#endif

#include "low/canton_config.h"
//#include "block_canton.h"
#include "param.h"

#include "train.h"
#include "low/turnout_config.h"


// ---------------------------------------------------------

#ifndef NUM_LOCAL_CANTONS_SW
#define NUM_LOCAL_CANTONS_SW 1
#error ops
#endif


//#define ADD_DUMMY_CANTON

/*
#ifdef ADD_DUMMY_CANTON
#define NUM_CANTONS (NUM_LOCAL_CANTONS+1)
#else
#define NUM_CANTONS (NUM_LOCAL_CANTONS)
#endif
*/

#define NUM_CANTONS (NUM_LOCAL_CANTONS_SW)

#define NUM_TRAINS  1

#define NUM_TURNOUTS 1

// ---------------------------------------------------------

void railconfig_setup_default(void);


// ---------------------------------------------------------

const canton_config_t *get_canton_cnf(int idx);
//canton_vars_t   *get_canton_vars(int idx);
//int canton_idx(canton_vars_t *v);

//const block_canton_config_t *get_block_canton_cnf(int idx);
//block_canton_vars_t   *get_block_canton_vars(int idx);
//int block_canton_idx(block_canton_vars_t *v);

const train_config_t  *get_train_cnf(int idx);
//train_vars_t  *get_train_vars(int idx);
//int train_idx(train_vars_t *v);

const turnout_config_t  *get_turnout_cnf(int idx);
//turnout_vars_t  *get_turnout_vars(int idx);
//int turnout_idx(turnout_vars_t *v);


// ---------------------------------------------------------

/*
#define USE_BLOCK_CANTON(_idx) \
		const block_canton_config_t *bcconf = get_block_canton_cnf(_idx); \
		block_canton_vars_t         *bcvars = get_block_canton_vars(_idx);
*/
/*
#define USE_TURNOUT(_idx) \
		const turnout_config_t *aconf = get_turnout_cnf(_idx); \
		turnout_vars_t         *avars = get_turnout_vars(_idx);
*/
// ---------------------------------------------------------





#endif /* RAILCONFIG_H_ */
