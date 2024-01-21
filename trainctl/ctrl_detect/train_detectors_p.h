/*
 * train_detectors_p.h
 *
 *  Created on: Jul 31, 2023
 *      Author: danielbraun
 */

#ifndef CTRL_TRAIN_DETECTORS_P_H_
#define CTRL_TRAIN_DETECTORS_P_H_

int detect_step_check_canton_exist(xblkaddr_t);
int detect_step_notify_ui(xblkaddr_t);
int detect_step_start_pwm(xblkaddr_t);
int detect_step_wait_300ms(xblkaddr_t);
int detect_step_wait_report(xblkaddr_t);
//int detect_step_start_inameas(xblkaddr_t);
int detect_step_stop_pwm(xblkaddr_t);


/* stop actions steps */
int detect_step_stop_pwm(xblkaddr_t);
//int detect_step_stop_inameas(xblkaddr_t);
int detect_step_stop_notify_ui(xblkaddr_t);






#endif /* CTRL_TRAIN_DETECTORS_P_H_ */
