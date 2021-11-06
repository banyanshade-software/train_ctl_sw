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
    [res appendString:[self generateHeader]];
    [res appendString:[self generateSblks]];
    [res appendString:[self generateTurnouts]];
    [res appendString:[self generateFooter]];
    return res;
}

- (NSString *) generateHeader
{
    return @"<html><body style=\"background-color:lightgrey;\">hello<br/><svg height=\"300px\" id=\"svg_document\" width=\"640px\" version=\"1.1\" preserveAspectRatio=\"xMidYMid meet\" viewBox=\"0 0 640 300\"><g id=\"main_group\">\n";

}
- (NSString *) generateFooter
{
    return @"</g></svg></body></html>";
}
- (NSString *)generateTurnouts
{
    return @"";
}

- (NSString *)generateSblks
{
    if ((0)) return @" <polyline  id=\"BLK2\" points=\"130,15 130,122 177,165 342,165 \" stroke=\"#000000\" stroke-width=\"3px\" class=\"blk\" fill=\"none\" transform=\"\"></polyline>";
    NSMutableString *res = [[NSMutableString alloc]init];
    int n = topology_num_sblkd();
    for (int i=0; i<n; i++) {
        const topo_lsblk_t *s = topology_get_sblkd(i);
        if (!s) break;
        [res appendString:[self generateSblk:s num:i]];
    }
    return res;
}

#define SCL_X 30
#define SCL_Y 30

- (NSString *) generateSblk:(const topo_lsblk_t *)seg num:(int)segnum
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

@end
