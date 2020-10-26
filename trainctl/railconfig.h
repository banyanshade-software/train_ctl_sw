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
#include "canton.h"
#include "train.h"
#include "turnout.h"
#include "param.h"


// ---------------------------------------------------------

#define NUM_LOCAL_CANTONS 1
#define NUM_CANTONS (NUM_LOCAL_CANTONS+1)

#define NUM_TRAINS  1

#define NUM_TURNOUTS 1

// ---------------------------------------------------------

void railconfig_setup_default(void);


// ---------------------------------------------------------

const canton_config_t *get_canton_cnf(int idx);
canton_vars_t   *get_canton_vars(int idx);
int canton_idx(canton_vars_t *v);

const train_config_t  *get_train_cnf(int idx);
train_vars_t  *get_train_vars(int idx);
int train_idx(train_vars_t *v);

const turnout_config_t  *get_turnout_cnf(int idx);
turnout_vars_t  *get_turnout_vars(int idx);
int turnout_idx(turnout_vars_t *v);


// ---------------------------------------------------------

#define USE_TRAIN(_idx) \
		const train_config_t *tconf = get_train_cnf(_idx); \
		train_vars_t         *tvars = get_train_vars(_idx);

#define USE_CANTON(_idx) \
		const canton_config_t *cconf = get_canton_cnf(_idx); \
		canton_vars_t         *cvars = get_canton_vars(_idx);

#define USE_TURNOUT(_idx) \
		const turnout_config_t *aconf = get_turnout_cnf(_idx); \
		turnout_vars_t         *avars = get_turnout_vars(_idx);

// ---------------------------------------------------------



extern const param_t train_params[];


#endif /* RAILCONFIG_H_ */
