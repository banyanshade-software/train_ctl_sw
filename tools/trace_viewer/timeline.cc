/*
 * timeline.cc
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "timeline.h"
#include "../../trainctl/stm32/monitoring_def.h"


TimeLine::TimeLine(FILE *out, TraceTool *next) : TraceTool(out, next)
{
    memset(taskstate, ' ', sizeof(taskstate));
    taskstate[2*MAXTASK]='\0';
}

void TimeLine::finalReport(void)
{
}
void TimeLine::processEvent(trace_event_t *evt)
{
    char taskview[2*MAXTASK+1]; // max 16 tasks
    memcpy(taskview, taskstate, 2*MAXTASK+1);

    const char *evts = "??";
    int tick = 0;
    int task = evt->task;
    memcpy(taskview, taskstate, 2*MAXTASK+1);
    switch (evt->event) {
    case MONITOR_SW_IN:     evts = "SCHED IN   "; taskstate[2*task]=(task==4) ? 'x' : 'X'; taskview[2*task]='I'; break;
    case MONITOR_SW_OUT:    evts = "SCHED OUT  "; taskstate[2*task]='|'; taskview[2*task]='-'; break;
    case MONITOR_TICK:      evts = "TICK"; tick = 1; break;
    case MONITOR_READY:     evts = "READY      "; taskview[2*task]='R'; break;
    case MONITOR_NOTIF_WB:  evts = "NOTIF W BLK"; taskview[2*task]='W'; break;
    case MONITOR_NOTIF_W:   evts = "NOTIF W    "; taskview[2*task]='W'; break;
    case MONITOR_DELAY_U:   evts = "DELAY UNTIL"; taskview[2*task]='D'; break;
    case MONITOR_DELAY:     evts = "DELAY      "; taskview[2*task]='D'; break;
    case MONITOR_NOTIF:     evts = "NOTIF snd  "; break;
    case MONITOR_NOTIF_ISR: evts = "NOTIF s ISR"; break;
    default : break;
    }

    if (evt->ts>=0) fprintf(out, "%12.6f [%0.4f] : ", evt->ts, evt->dts);
    else printf("                      : ");
    if (tick) {
        printf("%8lu [%6lu] %s ------------------------------------ systick\n", evt->cycle, evt->dcycle, taskview);
    } else {
        printf("%8lu [%6lu] %s %s TASK%d\n", evt->cycle, evt->dcycle, taskview, evts, task);
    }
}

