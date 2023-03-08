/*
 * TaskLatency.cc
 *
 *  Created on: 27 févr. 2023
 *      Author: danielbraun
 */



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "TaskLatency.h"
#include "../../trainctl/stm32/monitoring_def.h"



TaskLatency::TaskLatency(FILE *out, TraceTool *nexttool, int _tasknum) : TraceToolTask(out, nexttool, _tasknum)
{
	readycycle = 0;
	statLatency = StatVal();
}

void TaskLatency::processEvent(trace_event_t *evt)
{
	// event are already filtered for this task
	if ((0)) {
	} else if (evt->event == MONITOR_READY) {
		if (readycycle) {
			fprintf(out, "missing events, task %d cycle=%llu\n", tasknum, readycycle);
			readycycle = 0;
		}
		assert(readycycle==0);
		readycycle = evt->cycle;
	} else if (evt->event == MONITOR_SW_IN) {
		if (readycycle) {
			unsigned long long v = evt->cycle - readycycle;
			readycycle = 0;
			statLatency.addValue(v);
		}
	}
}

void TaskLatency::finalReport(void)
{
	fprintf(out, "---- TaskLatency task%d -----\n", tasknum);
	statLatency.printStats(out);
}

