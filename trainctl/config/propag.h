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

// local master-only config
void *conf_local_ptr(unsigned int lconfnum);
unsigned int conf_local_size(unsigned int lconfnum);

int32_t conf_local_get(unsigned int confnum, unsigned int fieldnum, unsigned int instnum);
void conf_local_set(unsigned int confnum, unsigned int fieldnum, unsigned int instnum, int32_t v);

#endif /* CONFIG_PROPAG_H_ */
