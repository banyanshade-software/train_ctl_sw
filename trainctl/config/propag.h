/*
 * propag.h
 *
 *  Created on: Apr 14, 2022
 *      Author: danielbraun
 */

#ifndef CONFIG_PROPAG_H_
#define CONFIG_PROPAG_H_

void conf_propagate(unsigned int confnum, unsigned int fieldnum, unsigned int instnum, int32_t v);

int32_t conf_default_value(unsigned int confnum, unsigned int fieldnum, unsigned int confbrd, unsigned int instnum);


#endif /* CONFIG_PROPAG_H_ */
