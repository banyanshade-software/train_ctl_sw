/*
 * train_detector_params.h
 *
 *  Created on: Jan 20, 2024
 *      Author: danielbraun
 */

#ifndef CTRL_DETECT_TRAIN_DETECTORS_PARAMS_H_
#define CTRL_DETECT_TRAIN_DETECTORS_PARAMS_H_


/*
 * if defined, we skip first detector (basic presence)
 * and assume a train is present on S5(C2)
 */
#define _TEST_FREQ_ON_S5



// freq resp detector. timing in ms
#define FREQ_RELAX_TIME  20
#define FREQ_FIRST_STEP_DUR	 20
#define FREQ_STEP_DUR	 20
#define FREQ_NSTEPS		 8


#endif /* CTRL_DETECT_TRAIN_DETECTORS_PARAMS_H_ */
