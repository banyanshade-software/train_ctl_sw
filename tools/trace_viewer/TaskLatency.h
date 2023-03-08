/*
 * TaskLatency.h
 *
 *  Created on: 27 févr. 2023
 *      Author: danielbraun
 */

#ifndef TASKLATENCY_H_
#define TASKLATENCY_H_


#include "tracetool.h"



class TaskLatency : public TraceToolTask {
public:
	TaskLatency(FILE *out, TraceTool *nexttool, int _tasknum);
protected:
	void processEvent(trace_event_t *);
	void finalReport(void);

private:
	unsigned long long readycycle;
	StatVal statLatency;
};



#endif /* TASKLATENCY_H_ */
