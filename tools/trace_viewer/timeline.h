/*
 * timeline.h
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#ifndef TIMELINE_H_
#define TIMELINE_H_

#include "tracetool.h"


class TimeLine : public TraceTool {
public:
	TimeLine(FILE *stdout, TraceTool *nexttool);
protected:
	void processEvent(trace_event_t *);
	void finalReport(void);
private:
    char taskstate[2*MAXTASK+1];

};

#endif /* TRACETOOL_H_ */
