//
//  AppDelegate.m
//  SvgGen
//
//  Created by Daniel BRAUN on 06/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <libgen.h>
#import "AppDelegate.h"

#include "topologyP.h"
#include "topology.h"

#define MAX_POINTS 4
@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate {
    NSString *svgHtml;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    [self generateAndDisplaySvg:nil];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (IBAction) generateAndDisplaySvg:(id)sender
{
    svgHtml = [self generateSvg];
    if ((1)) {
        [_ctoWebView loadHTMLString:svgHtml baseURL:nil];
    } else {
        NSURL *u = [NSURL URLWithString:@"http://wwww.google.com"];
        NSURLRequest *r = [NSURLRequest requestWithURL:u];
        [_ctoWebView loadRequest:r];
    }
    [_ctoWebView setNeedsLayout:YES];
    [_ctoWebView setNeedsDisplay:YES];
}
- (NSString *) generateSvg
{
    NSMutableString *res = [[NSMutableString alloc]init];
    NSMutableString *resG = [[NSMutableString alloc]init];
    NSMutableString *resT = [[NSMutableString alloc]init];
    [res appendString:[self generateHeader]];
    [self generateSblksInG:resG T:resT];
    [self generateTurnoutsInG:resG T:resT];
    [res appendString:@"<g id=\"main_group\">"];
    [res appendString:resG];
    [res appendString:@"</g>\n"];
    [res appendString:resT];
    [res appendString:[self generateFooter]];
    return res;
}

- (NSString *) generateHeader
{
    return @"<html><body style=\"background-color:lightgrey;\"><svg height=\"500px\" id=\"svg_document\" width=\"800px\" version=\"1.1\" preserveAspectRatio=\"xMidYMid meet\" viewBox=\"0 0 800 500\">\n";

}
- (NSString *) generateFooter
{
    return @"</g></svg></body></html>\n";
}


#define SCL_X 40
#define SCL_Y 40

- (void)generateSblksInG:(NSMutableString *)resG T:(NSMutableString *)resT
{
    int n = topology_num_sblkd();
    for (int i=0; i<n; i++) {
        const topo_lsblk_t *s = topology_get_sblkd(i);
        int fut = 0;
        if (s->canton_addr == 0xFF) {
            fut=1;
        }
        if (!s) break;
        [resG appendString:[self generateSblkPoly:s num:i]];
        int cx = ((s->points[s->p0].c + s->points[s->p0+1].c) * SCL_X / 2);
        int cy = ((s->points[s->p0].l + s->points[s->p0+1].l) * SCL_Y / 2);
        int v = 0;
        if (s->points[s->p0].c == s->points[s->p0+1].c) v=1;
        int tx,ty, rx, ry;
        if (v) {
            tx = cx-12;
            ty = cy-20;
            rx = cx + 4;
            ry = cy - 25;
        } else {
            tx = cx-20;
            ty = cy+12;
            rx = cx - 25;
            ry = cy - 19;
        }
        
        if (fut) {
            [resT appendFormat:@"<text x=\"%dpx\" y=\"%dpx\" class=\"label\" Font-family=\"Helvetica\" fill=\"#8080A0\" font-size=\"12px\" %s>f%d </text>\n",
             tx, ty, v ? "writing-mode=vertical-rl" : "",
             i];
        } else {
            [resT appendFormat:@"<text x=\"%dpx\" y=\"%dpx\" class=\"label\" Font-family=\"Helvetica\" fill=\"#8080A0\" font-size=\"12px\" %s>b%d (c%d)</text>\n",
            tx, ty, v ? "writing-mode=vertical-rl" : "",
               i, s->canton_addr];
        }
        //if (fut) continue;

        [resT appendFormat:@"<rect class=\"trinfo %s rectinfo%d trinfo_s%d trinfo_c%d\" x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#E0FFE0\" stroke=\"#80A080\" stroke-width=\"2px\" />\n",
         fut ? "fut" : "", i, i, s->canton_addr,
         rx,ry, v ? 16 : 50, v ? 50 : 16];
        [resT appendFormat:@"<text id=\"tr%d\" class=\"trinfo %s trinfo_s%d trinfo_c%d\" x=\"%dpx\" y=\"%dpx\"  Font-family=\"Helvetica\" fill=\"#8080A0\" font-size=\"10px\" %s>TRINFO</text>\n",
         i, fut ? "fut" : "", i, s->canton_addr,
         rx + (v? 8: 2), ry + (v ? 3 : 12), v ? "writing-mode=vertical-rl" : ""
         ];
    }
}


- (NSString *) generateSblkPoly:(const topo_lsblk_t *)seg num:(int)segnum
{
    int fut = 0;
    if (seg->canton_addr == 0xFF) {
        fut=1;
    }
    NSMutableString *res = [[NSMutableString alloc]init];
    if (!fut) {
        [res appendFormat:@"<polyline id=\"SBLK%2.2d\" class=\"track CANTON%d\" stroke=\"#000000\" stroke-width=\"5px\" fill=\"none\" points=\"",
            segnum, seg->canton_addr];
    } else {
        [res appendFormat:@"<polyline id=\"SBLK%2.2d\" class=\"track future\" stroke=\"#353585\" stroke-width=\"3px\" stroke-dasharray=\"8 2 2\" fill=\"none\" points=\"",
            segnum];
    }
    for (int i=0; i<MAX_POINTS; i++) {
        coord_t pt = seg->points[i];
        if (pt.l<0) break;
        [res appendFormat:@"%s%d,%d",
         i ? " ":"",
         pt.c*SCL_X, pt.l*SCL_Y];
    }
    [res appendString:@"\"></polyline>\n"];
    return res;
}
static int last_pt_idx(const topo_lsblk_t *s)
{
    for (int i=MAX_POINTS-1; i>=0; i--) {
        if (s->points[i].l >= 0) return i;
    }
    return 0;
}


typedef struct {
    coord_t p;
    int n;
    int l;
    int r;
    int straight_ep;
    int future;
} turn_rec_t;

typedef struct {
    turn_rec_t rec[3];
} trec_t;

static void store_rec_tn(trec_t *rec, coord_t pt, int left, int straight_endpoint, int future)
{
    for (int i=0; i<3; i++) {
        if (rec->rec[i].n) {
            if ((rec->rec[i].p.l == pt.l) && (rec->rec[i].p.c == pt.c)) {
                rec->rec[i].n++;
                if (left) rec->rec[i].l = 1;
                else rec->rec[i].r = 1;
                rec->rec[i].straight_ep = straight_endpoint;
                rec->rec[i].future = future;
                return;
            }
        } else {
            rec->rec[i].n = 1;
            rec->rec[i].p = pt;
            if (left) rec->rec[i].l = 1;
            else rec->rec[i].r = 1;
            rec->rec[i].straight_ep = straight_endpoint;
            rec->rec[i].future = future;
            return;
        }
    }
    abort();
}

static int rec_is_left(trec_t *rec)
{
    int nt=0;
    int nl=0;
    int nr=0;
    for (int i=0; i<3; i++) {
        int n = rec->rec[i].n;
        if (n!=1) abort();
        nt += n;
        nl += rec->rec[i].l;
        nr += rec->rec[i].r;
    }
    if (nt != 3) abort();
    if ((nl==2) && (nr==1)) return 1;
    if ((nr==2) && (nl==1)) return 0;
    abort();
    return -1;
}

static coord_t rec_get_dir(trec_t *rec, int n, int left, int *pstraight, int *pfuture)
{
    for (int i=0; i<3; i++) {
        if (left && !rec->rec[i].l) continue;
        if (!left && !rec->rec[i].r) continue;
        if (n) {
            n--;
            continue;
        }
        if (pstraight) *pstraight = rec->rec[i].straight_ep;
        if (pfuture) *pfuture = rec->rec[i].future;
        return rec->rec[i].p;
    }
    abort();
    return rec->rec[0].p;
}
static coord_t rec_get_left(trec_t *rec, int n, int *pfut)
{
    return rec_get_dir(rec, n, 1, NULL, pfut);
}

static coord_t rec_get_right(trec_t *rec, int n, int *pfut)
{
    return rec_get_dir(rec, n, 0, NULL, pfut);
}

static int rec_right_is_straight_ep(trec_t *rec, int n)
{
    int sep;
    rec_get_dir(rec, n, 0, &sep, NULL);
    return sep;
}
static int rec_left_is_straight_ep(trec_t *rec, int n)
{
    int sep;
    rec_get_dir(rec, n, 1, &sep, NULL);
    return sep;
}

- (void)generateTurnoutsInG:(NSMutableString *)resG T:(NSMutableString *)resT
{
    for (int tn=0; tn<128; tn++) {
        [self generateOneTurnoutInG:(NSMutableString *)resG T:(NSMutableString *)resT num:tn isDoor:NO];
    }
    for (int tn=128; tn<255; tn++) {
        [self generateOneTurnoutInG:(NSMutableString *)resG T:(NSMutableString *)resT num:tn isDoor:YES];
    }
}
- (void)generateOneTurnoutInG:(NSMutableString *)resG T:(NSMutableString *)resT num:(int)tn isDoor:(BOOL)door
{
    int ns = topology_num_sblkd();
    // get turnout coord
    trec_t rec;
    memset(&rec, 0, sizeof(rec));
    int minx=999; int maxx=-1; int miny=999; int maxy=-1;
    for (int b=0; b<ns; b++) {
        const topo_lsblk_t *s = topology_get_sblkd(b);
        if (s->ltn == tn) {
            store_rec_tn(&rec, s->points[0], 1, (s->left2==-1), (s->canton_addr==0xFF));
            minx = MIN(minx, s->points[0].c);
            maxx = MAX(maxx, s->points[0].c);
            miny = MIN(miny, s->points[0].l);
            maxy = MAX(maxy, s->points[0].l);
        }
        if (s->rtn == tn) {
            int li = last_pt_idx(s);
            store_rec_tn(&rec, s->points[li], 0, (s->right2==-1), (s->canton_addr==0xFF));
            minx = MIN(minx, s->points[li].c);
            maxx = MAX(maxx, s->points[li].c);
            miny = MIN(miny, s->points[li].l);
            maxy = MAX(maxy, s->points[li].l);
        }
    }
    if (minx==999) return;
    // turnout
    
    int x1,y1,x2,y2;
    int fut1, fut2;
    int gfut = 0;
    if (door) {
        x1 = rec_get_right(&rec,0, &fut1).c*SCL_X;
        y1 = rec_get_right(&rec,0, NULL).l*SCL_Y;
        x2 = rec_get_left(&rec,0, &fut2).c*SCL_X;
        y2 = rec_get_left(&rec,0, NULL).l*SCL_Y;
        int futa = fut1 | fut2;
        NSString *dash = futa ? @"stroke-dasharray=\"2 1\"" : @"";
        NSString *idpref = futa ? @"fut_" : @"";
        [resG appendFormat:@"<polyline polyline id=\"%@to%d%c\" class=\"turnout\" stroke=\"#000000\" stroke-width=\"1px\" fill=\"none\" points=\"%d,%d %d,%d\" %@></polyline>\n", idpref, tn,
         's',
         x1, y1, x2, y2, dash];
        
        int m = (x1+x2)/2;
        int h2 = (x2-x1)/3;
        [resG appendFormat:@"<polyline polyline id=\"%@to%d%c\" class=\"turnout\" stroke=\"#000000\" stroke-width=\"1px\" fill=\"none\" points=\"%d,%d %d,%d\" %@></polyline>\n", idpref, tn,
         't',
         m, y1-h2, m, y1+h2, dash];

        
        gfut = futa;
        
    } else {
        int isl = rec_is_left(&rec);
        
        if (isl) {
            x1 = rec_get_right(&rec,0, &fut1).c*SCL_X;
            y1 = rec_get_right(&rec,0, NULL).l*SCL_Y;
            x2 = rec_get_left(&rec,0, &fut2).c*SCL_X;
            y2 = rec_get_left(&rec,0, NULL).l*SCL_Y;
            int futa = fut1 | fut2;
            NSString *dash = futa ? @"stroke-dasharray=\"2 1\"" : @"";
            NSString *idpref = futa ? @"fut_" : @"";
            [resG appendFormat:@"<polyline polyline id=\"%@to%d%c\" class=\"turnout\" stroke=\"#000000\" stroke-width=\"1px\" fill=\"none\" points=\"%d,%d %d,%d\" %@></polyline>\n", idpref, tn,
             rec_left_is_straight_ep(&rec, 0) ? 's' : 't',
             x1, y1, x2, y2, dash];
            
            x1 = rec_get_right(&rec,0, &fut1).c*SCL_X;
            y1 = rec_get_right(&rec,0, NULL).l*SCL_Y;
            x2 = rec_get_left(&rec,1, &fut2).c*SCL_X;
            y2 = rec_get_left(&rec,1, NULL).l*SCL_Y;
            int futb = fut1 | fut2;
            dash = futb ? @"stroke-dasharray=\"2 1\"" : @"";
            idpref = futb ? @"fut_" : @"";
            [resG appendFormat:@"<polyline polyline id=\"%@to%d%c\" class=\"turnout\" stroke=\"#000000\" stroke-width=\"1px\" fill=\"none\" points=\"%d,%d %d,%d\" %@></polyline>\n", idpref, tn,
             rec_left_is_straight_ep(&rec, 1) ? 's' : 't',
             x1, y1, x2, y2, dash];
            gfut = futa | futb;
        } else {
            x1 = rec_get_left(&rec,0, &fut1).c*SCL_X;
            y1 = rec_get_left(&rec,0, NULL).l*SCL_Y;
            x2 = rec_get_right(&rec,0, &fut2).c*SCL_X;
            y2 = rec_get_right(&rec,0, NULL).l*SCL_Y;
            int futa = fut1 | fut2;
            NSString *dash = futa ? @"stroke-dasharray=\"4 2\"" : @"";
            NSString *idpref = futa ? @"fut_" : @"";
            [resG appendFormat:@"<polyline polyline id=\"%@to%d%c\" class=\"turnout\" stroke=\"#000000\" stroke-width=\"1px\" fill=\"none\" points=\"%d,%d %d,%d\" %@></polyline>\n", idpref, tn,
             rec_right_is_straight_ep(&rec, 0) ? 's' : 't',
             x1, y1, x2, y2, dash];
            
            x1 = rec_get_left(&rec,0, &fut1).c*SCL_X;
            y1 = rec_get_left(&rec,0, NULL).l*SCL_Y;
            x2 = rec_get_right(&rec,1, &fut2).c*SCL_X;
            y2 = rec_get_right(&rec,1, NULL).l*SCL_Y;
            int futb = fut1 | fut2;
            dash = futb ? @"stroke-dasharray=\"4 2\"" : @"";
            idpref = futb ? @"fut_" : @"";
            [resG appendFormat:@"<polyline polyline id=\"%@to%d%c\" class=\"turnout\" stroke=\"#000000\" stroke-width=\"1px\" fill=\"none\" points=\"%d,%d %d,%d\" %@></polyline>\n", idpref, tn,
             rec_right_is_straight_ep(&rec, 1) ? 's' : 't',
             x1, y1, x2, y2, dash];
            gfut = futa | futb;
        }
        
    }

    
    NSString *dash = gfut ? @"stroke-dasharray=\"1 2\"" : @"";
    NSString *idpref = gfut ? @"fut_" : @"";
    
    [resT appendFormat:@"<circle class=\"%@tncircle\" id=\"c%d\" cx=\"%d\" cy=\"%d\" r=\"%d\" stroke=\"#306030\" stroke-width=\"1px\" fill=\"#00000000\" %@/>\n", idpref, tn,
     SCL_X*(maxx+minx)/2, SCL_Y* (maxy+miny)/2, (SCL_X+SCL_Y)/3, dash];
    [resT appendFormat:@"<text x=\"%dpx\" y=\"%dpx\" class=\"turnout\" Font-family=\"Helvetica\" fill=\"#80A080\" font-size=\"12px\">S%d</text>\n",
     SCL_X*(maxx+minx)/2-8, SCL_Y*(maxy+miny)/2+SCL_Y/2+16,
     tn];
    
    NSLog(@"hop");
    
}

- (NSURL *)savePath
{
    char *s = dirname(__FILE__);
    NSString *p = [NSString stringWithFormat:@"%s/../../../trainctl/uitrack/uitrack.html", s];
    NSURL *u = [NSURL fileURLWithPath:p];
    return u;
}

- (IBAction) saveToFile:(id)sender
{
    NSURL *savepath = [self savePath];
    NSError *err = nil;
    BOOL ok = [svgHtml writeToURL:savepath atomically:YES encoding:NSUTF8StringEncoding error:&err];
    if (!ok) {
        NSLog(@"write error : %@", err);
    }
}




int occupency_turnout_reserve(uint8_t turnout, int8_t numtrain)
{
    return 0;
}
@end
