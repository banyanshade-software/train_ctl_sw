/*
 * globalcpu.h
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#ifndef GLOBALCPU_H_
#define GLOBALCPU_H_

#include "tracetool.h"


class GlobalCpu : public TraceTool {
public:
	GlobalCpu(FILE *out, TraceTool *nexttool);
protected:
	virtual bool filterEvent(trace_event_t *);
	void processEvent(trace_event_t *);
	void finalReport(void);

private:
	int curtask;
	unsigned long long begcycle;
	unsigned long long firstcycle;
	unsigned long long lastcycle;
	unsigned long long totcycle[MAXTASK];
};


#endif /* GLOBALCPU_H */
