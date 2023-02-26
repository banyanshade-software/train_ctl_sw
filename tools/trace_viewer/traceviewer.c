#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../trainctl/stm32/monitoring_def.h"

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
    char *fields[6];
    char line[1024];
    while (fgets(line, 1024, stdin)) {
        //printf("---- %s\n", line);
        int n = extract_fields(line, fields, 6);
        if (n!=6) {
            nskip++;
            continue;
        }
        nlines++;
        //printf("n=%d num %s port %s\n", n, fields[0], fields[1]);

        //"   33";"ITM Port 1";"260";"107404534";"2,237594 s";"Timestamp delayed. Packet delayed. "
        double ts;
        if (fields[4][0]=='?') {
            ts = -1;
        } else {
            //printf("ts:%s\n", fields[4]);
            // fix decimal point
            for (char *p = fields[4]; *p; p++) {
                if (','==*p) *p = '.';
            }
            ts = atof(fields[4]);
        }
        unsigned long cycle = atoi(fields[3]);
        unsigned int evt = atoi(fields[2]);
        unsigned int evt_type = evt >> 8;
        unsigned int task = evt & 0xFF;
        char *evts = "??";
        int tick = 0;
        switch (evt_type) {
        case MONITOR_SW_IN:     evts = "SCHED IN   "; break;
        case MONITOR_SW_OUT:    evts = "SCHED OUT  "; break;
        case MONITOR_TICK:      evts = "TICK"; tick = 1; break;
        case MONITOR_READY:     evts = "READY      "; break;
        case MONITOR_NOTIF_WB:  evts = "NOTIF W BLK"; break;
        case MONITOR_NOTIF_W:   evts = "SCHED W    "; break;
        case MONITOR_DELAY_U:   evts = "DELAY UNTIL"; break;
        case MONITOR_DELAY:     evts = "DELAY      "; break;
        case MONITOR_NOTIF:     evts = "NOTIF snd  "; break;
        case MONITOR_NOTIF_ISR: evts = "NOTIF s ISR"; break;
        default : break;
        }
        if (ts>=0) printf("%12.6f : ", ts);
        else printf("             : ");
        if (tick) {
            printf("%8lu ------------------------------------ systick\n", cycle);
        } else {
            printf("%8lu %s TASK%d\n", cycle, evts, task);
        }
    }
    printf("scanned %d, skipped %d\n", nlines, nskip);
}
