#ifndef TRAIN_SIMU_H
#define TRAIN_SIMU_H


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

void train_simu_canton_volt(int numcanton, int voltidx, int vlt100);
void train_simu_canton_set_pwm(int numcanton, int dir, int duty);

#define NUM_LOCAL_CANTONS 5

#endif
