/*
 * tracetool.c
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tracetool.h"
#include "../../trainctl/stm32/monitoring_def.h"


TraceTool::TraceTool(FILE *output, TraceTool *nexttool)
{
	out = output ? output : stdout;
	next = nexttool;
}

void TraceTool::handleEvent(trace_event_t *evt)
{
	if (filterEvent(evt)) processEvent(evt);
	if (next) next->handleEvent(evt);
}

bool TraceTool::filterEvent(trace_event_t *evt)
{
	return true;
}
void TraceTool::printReport(void)
{
	fprintf(out, "\n\n=============== reports ================\n");
	finalReport();
	if (next) next->finalReport();
}

// ------------


TraceToolTask::TraceToolTask(FILE *output, TraceTool *nexttool, int _tasknum) : TraceTool(output, nexttool)
{
	tasknum = _tasknum;
}
bool TraceToolTask::filterEvent(trace_event_t *evt)
{
	if (evt->task == tasknum) return true;
	return false;
}

// ------------------------------------------------------------------------------------------------


// ------------
