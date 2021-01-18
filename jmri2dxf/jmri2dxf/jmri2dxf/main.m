//
//  main.m
//  jmri2dxf
//
//  Created by Daniel BRAUN on 16/01/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <Foundation/Foundation.h>
//#import <stdlib.h>
#import "dxfout.h"


@interface JMRIpoint : NSObject {
@public
    double x;
    double y;
}
+ (instancetype)pointWithX:(double)x Y:(double)y;
@end

@implementation JMRIpoint

+ (instancetype)pointWithX:(double)x Y:(double)y
{
    JMRIpoint *p = [[JMRIpoint alloc]init];
    p->x = x;
    p->y = y;
    return p;
}


@end

static void *dxf_init(const char *fn);
static void dxf_end(void *);
static void dxf_line(void *, double, double, double, double);
static void dxf_arc(void *, double, double, double, double, double);

static void parseJmri(const char *fn)
{
    NSError *err;
    NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fn]];
    NSXMLDocument *d = [[NSXMLDocument alloc] initWithContentsOfURL:url
                                                      options:0 error:&err];
    if (!d) {
        NSLog(@"can't read file : %@\n", err);
        exit(1);
    }
    NSString *s = [NSString stringWithUTF8String:fn];
    s = [s stringByDeletingPathExtension];
    s = [s stringByAppendingPathExtension:@"dxf"];
    void *dxf = dxf_init([s UTF8String]);
    if (!dxf) {
        NSLog(@"can't write dxf file %@", s);
        exit(2);
    }
    NSArray *ea = [d nodesForXPath:@"/layout-config/LayoutEditor" error:nil];
    if ([ea count] != 1) {
        // or can we have several LayoutEditor ??
        NSLog(@"bad JMRI file\n");
        exit(3);
    }
    NSXMLNode *le = [ea firstObject];
    NSMutableDictionary *points = [NSMutableDictionary dictionaryWithCapacity:100];
    NSArray *ptxml = [le nodesForXPath:@"positionablepoint" error:nil];
    for (NSXMLElement *e in ptxml) {
        NSString *ident = [[e  attributeForName:@"ident"] stringValue];
        double x = [[[e  attributeForName:@"x"]objectValue]doubleValue];
        double y = [[[e  attributeForName:@"y"]objectValue]doubleValue];
        //NSLog(@"point %@", ident);
        JMRIpoint *pt = [JMRIpoint pointWithX:x Y:y];
        [points setObject:pt forKey:ident];
    }
    NSMutableDictionary *turnouts = [[NSMutableDictionary alloc]initWithCapacity:10];
    NSArray *toxml = [le nodesForXPath:@"layoutturnout" error:nil];
    for (NSXMLElement *e in toxml) {
        NSString *ident = [[e  attributeForName:@"ident"] stringValue];
        [turnouts setObject:@1 forKey:ident];
        for (char c = 'a'; c<='d'; c++) {
            double x = [[[e  attributeForName:[NSString stringWithFormat:@"x%c", c]]objectValue]doubleValue];
            double y = [[[e  attributeForName:[NSString stringWithFormat:@"y%c", c]]objectValue]doubleValue];
            JMRIpoint *pt = [JMRIpoint pointWithX:x Y:y];
            [points setObject:pt forKey:[NSString stringWithFormat:@"%@_%c", ident, c]];
        }
        double x = [[[e  attributeForName:@"xcen"]objectValue]doubleValue];
        double y = [[[e  attributeForName:@"ycen"]objectValue]doubleValue];
        JMRIpoint *pt = [JMRIpoint pointWithX:x Y:y];
        [points setObject:pt forKey:[NSString stringWithFormat:@"%@", ident]];
        //NSLog(@"point %@", ident);
    }

    NSArray *tsxml = [le nodesForXPath:@"tracksegment" error:nil];
    for (NSXMLElement *e in tsxml) {
        NSString *ident = [[e attributeForName:@"ident"]objectValue];
        NSString *arc = [[e attributeForName:@"arc"]objectValue];
        double angle = [[[e attributeForName:@"angle"]objectValue]doubleValue];
        NSString *spt1 = [[e attributeForName:@"connect1name"]stringValue];
        NSString *spt2 = [[e attributeForName:@"connect2name"]stringValue];
        NSLog(@"seg arc = %@  %@-%@ angle %f", arc, spt1, spt2, angle);
        JMRIpoint *pt1 = [points objectForKey:spt1];
        JMRIpoint *pt2 = [points objectForKey:spt2];
        if (!pt1) {
            if ([turnouts objectForKey:spt1]) {
                int n = [[[e attributeForName:@"type1"]objectValue]intValue];
                if (n) {
                    spt1 = [NSString stringWithFormat:@"%@_%c", spt1, n-1+'a'];
                    pt1 = [points objectForKey:spt1];
                }
            }
        }
        if (!pt2) {
            if ([turnouts objectForKey:spt2]) {
                int n = [[[e attributeForName:@"type2"]objectValue]intValue];
                if (n) {
                    spt2 = [NSString stringWithFormat:@"%@_%c", spt2, n-1+'a'];
                    pt2 = [points objectForKey:spt2];
                }
            }
        }
        if (!pt1 || !pt2) {
            NSLog(@"error point not found for seg %@ %@-%@\n", ident, spt1, spt2);
            continue;
            //exit(4);
        }
        if ([spt1 isEqual:spt2]) {
            NSLog(@"void segment");
            continue;
        }
        if (arc) {
            dxf_line(dxf, pt1->x, pt1->y, pt2->x, pt2->y);
            //dxf_arc(dxf, pt1->x, pt1->y, pt2->x, pt2->y, angle);
        } else {
            dxf_line(dxf, pt1->x, pt1->y, pt2->x, pt2->y);
        }
    }
    if ((1)) {
        dxf_arc(dxf,  300, 300, 80, 0, 90);
        dxf_arc(dxf,  0, 200, 180, 60, 120);
        dxf_arc(dxf,  340, 300, 80, 30, 90);
    }
    dxf_end(dxf);
}


int main(int argc, const char * argv[]) {
    const char *fn;
    if (argc != 2) {
        if ((1)) {
            //fn = "/Users/danielbraun/Documents/Hasna5.xml";
            fn = "/Users/danielbraun/Documents/test.xml";
        } else {
            fprintf(stderr, "usage : %s jmri_xml_file\n", argv[0]);
            exit(1);
        }
    } else {
        fn = argv[1];
    }
    @autoreleasepool {
        // insert code here...
        parseJmri(fn);
    }
    return 0;
}

#pragma mark -

#define LAYER "Layer"
#define COLOR 2

#define YSIGN (-1)
static void *dxf_init(const char *fn)
{
    FILE *F = fopen(fn, "w");
    if (!F) {
        NSLog(@"cant create %s\n", fn);
        exit(11);
    }
    DXF_Begin(F);
    return F;
}
static void dxf_end(void *h)
{
    FILE *F = (FILE *)h;
    DXF_End(F);
    fclose(F);
}
static void dxf_line(void *h, double x1, double y1, double x2, double y2)
{
    FILE *F = (FILE *)h;
    DXF_Point(F, LAYER, COLOR, x1, YSIGN*y1, 0);
    DXF_Point(F, LAYER, COLOR, x2, YSIGN*y2, 0);
    DXF_Line(F, LAYER, COLOR, x1, YSIGN*y1, 0, x2, YSIGN*y2, 0);
}

static void dxf_arc(void *h, double x1, double y1, double r, double ang0, double ang1)
{
    FILE *F = (FILE *)h;
    DXF_Point(F, LAYER, COLOR, x1, YSIGN*y1, 0);
    //DXF_Point(F, LAYER, COLOR, x2, YSIGN*y2, 0);
    //DXF_Line(F, LAYER, COLOR, x1, YSIGN*y1, 0, x2, YSIGN*y2, 0);
    DXF_Arc(F, LAYER, COLOR, x1, YSIGN*y1, 0, r, ang0, ang1);
}


