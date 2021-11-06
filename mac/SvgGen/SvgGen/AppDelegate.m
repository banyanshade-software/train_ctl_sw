//
//  AppDelegate.m
//  SvgGen
//
//  Created by Daniel BRAUN on 06/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import "AppDelegate.h"

#include "topologyP.h"

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
    return @"<html><body style=\"background-color:lightgrey;\">hello<br/><svg height=\"300px\" id=\"svg_document\" width=\"700px\" version=\"1.1\" preserveAspectRatio=\"xMidYMid meet\" viewBox=\"0 0 700 300\">\n";

}
- (NSString *) generateFooter
{
    return @"</g></svg></body></html>";
}


#define SCL_X 40
#define SCL_Y 40

- (void)generateSblksInG:(NSMutableString *)resG T:(NSMutableString *)resT
{
    //if ((0)) return @" <polyline  id=\"BLK2\" points=\"130,15 130,122 177,165 342,165 \" stroke=\"#000000\" stroke-width=\"3px\" class=\"blk\" fill=\"none\" transform=\"\"></polyline>";
    NSMutableString *res = [[NSMutableString alloc]init];
    int n = topology_num_sblkd();
    for (int i=0; i<n; i++) {
        const topo_lsblk_t *s = topology_get_sblkd(i);
        if (!s) break;
        [resG appendString:[self generateSblkPoly:s num:i]];
        int tx = ((s->points[0].c + s->points[1].c) * SCL_X / 2)-20;
        int ty = ((s->points[0].l + s->points[1].l) * SCL_Y / 2)-12;
        [resT appendFormat:@"<text x=\"%dpx\" y=\"%dpx\" class=\"label\" Font-family=\"Helvetica\" fill=\"#8080A0\" font-size=\"12px\">b%d (c%d)</text>\n",
         tx, ty, i, s->canton_addr];
    }
}


- (NSString *) generateSblkPoly:(const topo_lsblk_t *)seg num:(int)segnum
{
    NSMutableString *res = [[NSMutableString alloc]init];
    [res appendFormat:@"<polyline id=\"SBLK%2.2d\" class=\"CANTON%d\" stroke=\"#000000\" stroke-width=\"3px\" fill=\"none\" points=\"",
     segnum, seg->canton_addr];
    for (int i=0; i<MAX_POINTS; i++) {
        coord_t pt = seg->points[i];
        if (pt.l<0) break;
        [res appendFormat:@"%s%d,%d",
         i ? " ":"",
         pt.c*SCL_X, pt.l*SCL_Y];
    }
    [res appendString:@"\"></polyline>"];
    return res;
}
static int last_pt_idx(const topo_lsblk_t *s)
{
    for (int i=MAX_POINTS-1; i>=0; i--) {
        if (s->points[i].l >= 0) return i;
    }
    return 0;
}
- (void)generateTurnoutsInG:(NSMutableString *)resG T:(NSMutableString *)resT
{
    int ns = topology_num_sblkd();
    for (int tn=0; tn<16; tn++) {
        // get turnout coord
        int minx=999; int maxx=-1; int miny=999; int maxy=-1;
        for (int b=0; b<ns; b++) {
            const topo_lsblk_t *s = topology_get_sblkd(b);
            if (s->ltn == tn) {
                minx = MIN(minx, s->points[0].c);
                maxx = MAX(maxx, s->points[0].c);
                miny = MIN(miny, s->points[0].l);
                maxy = MAX(maxy, s->points[0].l);
            }
            if (s->rtn == tn) {
                int li = last_pt_idx(s);
                minx = MIN(minx, s->points[li].c);
                maxx = MAX(maxx, s->points[li].c);
                miny = MIN(miny, s->points[li].l);
                maxy = MAX(maxy, s->points[li].l);
            }
        }
        if (minx==999) continue;
        // hop
        NSLog(@"hop");
    }
}
@end
