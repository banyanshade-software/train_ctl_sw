/*
 * traceanalyser.h
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#ifndef TRACETOOL_H_
#define TRACETOOL_H_

typedef struct {
	unsigned long cycle;
	unsigned long dcycle;
	double ts;
	double lastts;
	double dts;
	unsigned int event;
	unsigned int task;
} trace_event_t;

#define MAXTASK 16

/*
 * abstract class for all tools
 */

class TraceTool {
public:
	TraceTool(FILE *output, TraceTool *nexttool);
	void printReport(void);
	void handleEvent(trace_event_t *);

	TraceTool *next;
protected:
	virtual void processEvent(trace_event_t *) = 0;
	virtual bool filterEvent(trace_event_t *);
	virtual void finalReport(void) = 0;
	bool isIdleTask(unsigned int tasknum);

	FILE *out;
private:
};

class TraceToolTask : public TraceTool {
public:
	TraceToolTask(FILE *output, TraceTool *nexttool, int tasknum);
protected:
	virtual bool filterEvent(trace_event_t *);
	int tasknum;
private:
};


class StatVal {
public:
	StatVal();
	void addValue(unsigned long long v);
	void printStats(FILE *out);
protected:
	unsigned int nval;
	unsigned long long min;
	unsigned long long max;
	unsigned long long total;
};


#endif /* TRACETOOL_H_ */
