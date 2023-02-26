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

#define MAXTASK 32

/*
 * abstract class for all tools
 */

class TraceTool {
public:
	TraceTool(FILE *output);
	virtual void finalReport(void) = 0;
	void handleEvent(trace_event_t *);

	TraceTool *next;
protected:
	virtual void processEvent(trace_event_t *) = 0;
	virtual bool filterEvent(trace_event_t *);
	bool isIdleTask(unsigned int tasknum);

	FILE *out;
private:
};

class TraceToolTask : public TraceTool {
public:
	TraceToolTask(FILE *output, int tasknum);
protected:
	virtual bool filterEvent(trace_event_t *);
	int tasknum;
private:
};



class GlobalCpu : public TraceTool {
public:
	GlobalCpu(FILE *out);
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


#endif /* TRACETOOL_H_ */
