#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../trainctl/stm32/monitoring_def.h"

#include "tracetool.h"
#include "timeline.h"
#include "globalcpu.h"
#include "TaskLatency.h"

/*
"   32";"ITM Port 1";"515";"107404534 ?";"?";"No timestamp received for packet, cycles value guessed. "
"   33";"ITM Port 1";"260";"107404534";"2,237594 s";"Timestamp delayed. Packet delayed. "
"   34";"ITM Port 1";"768";"107448694";"2,238514 s";""
"   35";"ITM Port 1";"768";"107496693";"2,239514 s";""
"   36";"ITM Port 1";"768";"107544692";"2,240514 s";""
*/


static int extract_fields(char *line, char **fields, int n)
{
    int st = 0;
    int fidx = 0;
    for (char *p = line; *p; p++) {
        switch(st) {
        case 0: // waiting for beginning "
            if (*p == '"') {
                st = 1;
                fields[fidx] = p+1;
            }
            break;
        case 1: // in field, waiting for end "
            if (*p == '"') {
                *p = '\0';
                st = 2;
                fidx++;
            }
            break;
        case 2: // end of field, waiting for ;
            if (*p == ';') {
                st = 0;
            }
            break;
        }
    }
    return fidx;
}


int main(int argc, char **argv)
{
    int nlines = 0;
    int nskip = 0;
    int noch = 0;
    char *fields[6];
    char line[1024];

    unsigned long lastcycle = 0;
    double lastts = 0.0;

    TraceTool *tools = new GlobalCpu(stdout, NULL);
    tools = new TaskLatency(stdout, tools, 1);
    tools = new TaskLatency(stdout, tools, 2);
    tools = new TaskLatency(stdout, tools, 3);
    tools = new TaskLatency(stdout, tools, 4);
    tools = new TaskLatency(stdout, tools, 5);
    tools = new TimeLine(stdout, tools);

    while (fgets(line, 1024, stdin)) {
        //printf("---- %s\n", line);
        int n = extract_fields(line, fields, 6);
        if (n!=6) {
            nskip++;
            continue;
        }
        nlines++;

        if (strcmp(fields[1],"ITM Port 1"))  {
            noch++;
            continue;
        }

        trace_event_t event;
        event.dts = 0.0;


        if (fields[4][0]=='?') {
        	event.ts = -1;
        } else {
            // fix decimal point
            for (char *p = fields[4]; *p; p++) {
                if (','==*p) *p = '.';
            }
            event.ts = atof(fields[4]);
            if (lastts) {
            	event.dts = event.ts - lastts;
            }
            lastts = event.ts;
        }
        event.cycle = atoi(fields[3]);
        event.dcycle = 0;
        if (lastcycle) event.dcycle = event.cycle-lastcycle;
        lastcycle = event.cycle;
        unsigned int evt = atoi(fields[2]);
        event.event = evt >> 8;
        event.task = evt & 0xFF;


        if (tools) tools->handleEvent(&event);


    }
    if (tools) tools->printReport();

    printf("scanned %d, skipped %d, other ch %d\n", nlines, nskip, noch);
}
