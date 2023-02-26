/*
 * tracetool.c
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tracetool.h"
#include "../../trainctl/stm32/monitoring_def.h"


TraceTool::TraceTool(FILE *output)
{
	out = output ? output : stdout;
	next = NULL;
}

void TraceTool::handleEvent(trace_event_t *evt)
{
	if (filterEvent(evt)) processEvent(evt);
}

bool TraceTool::filterEvent(trace_event_t *evt)
{
	return true;
}

// ------------


TraceToolTask::TraceToolTask(FILE *output, int _tasknum) : TraceTool(output)
{
	tasknum = _tasknum;
}
bool TraceToolTask::filterEvent(trace_event_t *evt)
{
	if (evt->task == tasknum) return true;
	return false;
}

// ------------------------------------------------------------------------------------------------

GlobalCpu::GlobalCpu(FILE *out) : TraceTool(out)
{
	curtask = -2;
}

bool GlobalCpu::filterEvent(trace_event_t *evt)
{
	if (TraceTool::filterEvent(evt)) {
		if (evt->event == MONITOR_SW_IN) return true;
		if (evt->event == MONITOR_SW_OUT) return true;
	}
	return false;
}

void GlobalCpu::processEvent(trace_event_t *evt)
{
	lastcycle = evt->cycle;
	if (evt->event == MONITOR_SW_IN) {
		assert(curtask<0);
		if (-2==curtask) firstcycle = evt->cycle;
		curtask = evt->task;
		begcycle = evt->cycle;
	} else if (evt->event == MONITOR_SW_OUT) {
		if (curtask>=0) {
			totcycle[curtask] += evt->cycle - begcycle;
			curtask = -1;
		} else {
			// occurs if begin of trace does not includes the matching SW_IN
		}
	}
}

void GlobalCpu::finalReport(void)
{
	fprintf(out, "---- GlobalCpu -----\n");
	unsigned long long totalcycles = lastcycle - firstcycle;
	for (int t=0; t<MAXTASK; t++) {
		if (!totcycle[t]) continue;
		double v = 1.0*totcycle[t] / totalcycles;
		fprintf(out, "\ttask %d : %4.2f %%cpu\n", t, v*100);
	}
}

