#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../trainctl/stm32/monitoring_def.h"

#include "tracetool.h"
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
    char taskview[2*MAXTASK+1]; // max 16 tasks
    char taskstate[2*MAXTASK+1];
    memset(taskstate, ' ', sizeof(taskstate));
    taskstate[2*MAXTASK]='\0';
    memcpy(taskview, taskstate, 2*MAXTASK+1);
    unsigned long lastcycle = 0;
    double lastts = 0.0;

    TraceTool *tools = new GlobalCpu(stdout);

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

        //printf("n=%d num %s port %s\n", n, fields[0], fields[1]);

        //"   33";"ITM Port 1";"260";"107404534";"2,237594 s";"Timestamp delayed. Packet delayed. "
        double ts;
        double dts = 0.0;
        if (fields[4][0]=='?') {
            ts = -1;
        } else {
            //printf("ts:%s\n", fields[4]);
            // fix decimal point
            for (char *p = fields[4]; *p; p++) {
                if (','==*p) *p = '.';
            }
            ts = atof(fields[4]);
            if (lastts) {
                dts = ts - lastts;
            }
            lastts = ts; 
        }
        unsigned long cycle = atoi(fields[3]);
        unsigned long dcycle = 0;
        if (lastcycle) dcycle = cycle-lastcycle;
        lastcycle = cycle;
        unsigned int evt = atoi(fields[2]);
        unsigned int evt_type = evt >> 8;
        unsigned int task = evt & 0xFF;


        trace_event_t event;
        event.cycle = cycle;
        event.dcycle = dcycle;
        event.ts = ts;
        event.dts = ts-lastts;
        event.event = evt_type;
        event.task = task;

        if (tools) tools->handleEvent(&event);


        const char *evts = "??";
        int tick = 0;
        
        memcpy(taskview, taskstate, 2*MAXTASK+1);
        switch (evt_type) {
        case MONITOR_SW_IN:     evts = "SCHED IN   "; taskstate[2*task]=(task==4) ? 'x' : 'X'; taskview[2*task]='I'; break;
        case MONITOR_SW_OUT:    evts = "SCHED OUT  "; taskstate[2*task]='|'; taskview[2*task]='-'; break;
        case MONITOR_TICK:      evts = "TICK"; tick = 1; break;
        case MONITOR_READY:     evts = "READY      "; taskview[2*task]='R'; break;
        case MONITOR_NOTIF_WB:  evts = "NOTIF W BLK"; taskview[2*task]='W'; break;
        case MONITOR_NOTIF_W:   evts = "SCHED W    "; taskview[2*task]='W'; break;
        case MONITOR_DELAY_U:   evts = "DELAY UNTIL"; taskview[2*task]='D'; break;
        case MONITOR_DELAY:     evts = "DELAY      "; taskview[2*task]='D'; break;
        case MONITOR_NOTIF:     evts = "NOTIF snd  "; break;
        case MONITOR_NOTIF_ISR: evts = "NOTIF s ISR"; break;
        default : break;
        }
        if (ts>=0) printf("%12.6f [%0.4f] : ", ts, dts);
        else printf("                      : ");
        if (tick) {
            printf("%8lu [%6lu] %s ------------------------------------ systick\n", cycle, dcycle, taskview);
        } else {
            printf("%8lu [%6lu] %s %s TASK%d\n", cycle, dcycle, taskview, evts, task);
        }
    }
    if (tools) tools->finalReport();

    printf("scanned %d, skipped %d, other ch %d\n", nlines, nskip, noch);
}
