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

// TODO this file should be  removed and replaced by conf_*

#ifndef RAILCONFIG_H_
#define RAILCONFIG_H_

#include "trainctl_iface.h"

#ifdef TRAIN_SIMU
#include "train_simu.h"
#endif


#endif /* RAILCONFIG_H_ */
