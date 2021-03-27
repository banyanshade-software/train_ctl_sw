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

#define TRK_WIDTH 8

typedef struct vect {
    double x;
    double y;
} vector_t;

@interface JMRIpoint : NSObject {
@public
    struct vect p;
    //double x;
    //double y;
    
    int ndone; // if nx ny already set
    struct vect norm; // unit vector orthogonal to track at this point
}
+ (instancetype)pointWithX:(double)x Y:(double)y;
@end

@implementation JMRIpoint

+ (instancetype)pointWithX:(double)x Y:(double)y
{
    JMRIpoint *p = [[JMRIpoint alloc]init];
    p->p.x = x;
    p->p.y = y;
    return p;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%fx%f", self->p.x, self->p.y];
}
@end

// basic vector op
static vector_t vect_add(vector_t a, vector_t b)
{
    vector_t r;
    r.x = a.x+b.x;
    r.y = a.y+b.y;
    return r;
}
static vector_t vect_sub(vector_t a, vector_t b)
{
    vector_t r;
    r.x = a.x-b.x;
    r.y = a.y-b.y;
    return r;
}
static vector_t vect_normalize(vector_t a)
{
    double l = sqrt(a.x*a.x + a.y * a.y);
    if (!l) return a;
    a.x = a.x/l;
    a.y = a.y/l;
    return a;
}
static double vect_norm(vector_t a)
{
    double l = sqrt(a.x*a.x + a.y * a.y);
    return l;
}
static double vect_norm2(vector_t a)
{
    double l = a.x*a.x + a.y * a.y;
    return l;
}
static double vect_scalprod(vector_t a, vector_t b)
{
    double l = a.x*b.x + a.y * b.y;
    return l;
}
static vector_t vect_orth(vector_t a)
{
    vector_t r;
    r.x = -a.y;
    r.y = a.x;
    return r;
}

#pragma mark -


#pragma mark -

static void *dxf_init(const char *fn);
static void dxf_end(void *);
static void dxf_line(void *, double, double, double, double);
static void dxf_line2(void *, double, double, double, double);
static void dxf_line3(void *, double, double, double, double);
static void dxf_arc(void *, double, double, double, double, double);
static void dxf_arc2(void *, double, double, double, double, double);
static void dxf_arc3(void *, double, double, double, double, double);


static NSMutableDictionary *points = NULL;
static NSMutableDictionary *turnouts = NULL;
static void *dxf = NULL;

static int get_normal1_for_seg(JMRIpoint *p1, JMRIpoint *p2)
{
    vector_t n = vect_normalize(vect_orth(vect_sub(p2->p, p1->p)));
    if ((1)) {
        double d=vect_scalprod(n, vect_sub(p2->p, p1->p));
        if (d>0.01) abort();
    }
    if (p1->ndone) {
        // check same line
        double r1 = vect_norm2(vect_sub(n,p1->norm));
        double r2 = vect_norm2(vect_add(n,p1->norm));
        const double epsilon = 0.08;
        if ((r1>epsilon) && (r2>epsilon)) {
            NSLog(@"bad norm");
            return -1;
        } else {
            NSLog(@"norm ok");
        }
    } else {
        p1->norm = n;
        p1->ndone = 1;
        if (!p1->norm.x && !p1->norm.y) abort();
    }
    return 0;
}


static void find_circle2(JMRIpoint *pt1, JMRIpoint *pt2, vector_t *center, double *radius, double *ang1, double *ang2);
//static void find_circle1(JMRIpoint *pt1, JMRIpoint *pt2, vector_t *center, double *radius, double *ang1, double *ang2);


static int trackSegment(NSXMLElement *e, int dostraight, int docurve)
{
    NSString *ident = [[e attributeForName:@"ident"]objectValue];
    NSString *arc = [[e attributeForName:@"arc"]objectValue];
    double angle = [[[e attributeForName:@"angle"]objectValue]doubleValue];
    if (arc && !docurve) return 0;
    if (!arc && !dostraight) return 0;
    
    NSString *spt1 = [[e attributeForName:@"connect1name"]stringValue];
    NSString *spt2 = [[e attributeForName:@"connect2name"]stringValue];
    NSLog(@"seg arc = %@  %@-%@ angle %f", arc, spt1, spt2, angle);
    JMRIpoint *pt1 = [points objectForKey:spt1];
    JMRIpoint *pt2 = [points objectForKey:spt2];
    if (!pt1) {
        if ([turnouts objectForKey:spt1]) {
            int n = [[[e attributeForName:@"type1"]objectValue]intValue];
            if (n) {
                if (n<2) abort();
                spt1 = [NSString stringWithFormat:@"%@_%c", spt1, n-2+'a'];
                NSLog(@"seg %@ spt1=%@", ident, spt1);
                pt1 = [points objectForKey:spt1];
            }
        }
    }
    if (!pt2) {
        if ([turnouts objectForKey:spt2]) {
            int n = [[[e attributeForName:@"type2"]objectValue]intValue];
            // https://webserver.jmri.org/JavaDoc/doc/jmri/jmrit/display/layoutEditor/HitPointType.html
            if (n) {
                if (n<2) abort();
                spt2 = [NSString stringWithFormat:@"%@_%c", spt2, n-2+'a'];
                NSLog(@"seg %@ spt2=%@", ident, spt2);
                pt2 = [points objectForKey:spt2];
            }
        }
    }
    if (!pt1 || !pt2) {
        NSLog(@"error point not found for seg %@ %@-%@\n", ident, spt1, spt2);
        return 0;
        //exit(4);
    }
    if ([spt1 isEqual:spt2]) {
        NSLog(@"void segment");
        return 0;
    }
    if ((pt1->p.x == pt2->p.x) && (pt1->p.y == pt2->p.y)) {
        NSLog(@"void segment 2");
        return 0;
    }
    if (!arc) {
        get_normal1_for_seg(pt1, pt2);
        get_normal1_for_seg(pt2, pt1);
    }
    if (arc) {
        vector_t center;
        double radius;
        double ang1, ang2;
        double angle = [[[e attributeForName:@"angle"]objectValue]doubleValue];
        if (pt1->ndone && pt2->ndone) {
            NSLog(@"both normal");
            find_circle2(pt1, pt2, &center, &radius, &ang1, &ang2);
        } else if (pt1->ndone) {
            NSLog(@"pt1 normal");
            find_circle2(pt1, pt2, &center, &radius, &ang1, &ang2);
        } else if (pt2->ndone) {
            NSLog(@"pt2 normal");
            find_circle2(pt2, pt1, &center, &radius, &ang1, &ang2);
        } else {
            NSLog(@"NO normal");
            return 1;//1;
        }
        dxf_arc(dxf, center.x, center.y, radius, ang1, ang2);
        dxf_arc3(dxf, center.x, center.y, radius-TRK_WIDTH, ang1, ang2);
        dxf_arc3(dxf, center.x, center.y, radius+TRK_WIDTH, ang1, ang2);

        //dxf_line(dxf, pt1->p.x, pt1->p.y, pt2->p.x, pt2->p.y);
        //dxf_arc(dxf, pt1->x, pt1->y, pt2->x, pt2->y, angle);
    } else {
        dxf_line(dxf, pt1->p.x, pt1->p.y, pt2->p.x, pt2->p.y);
        if ((1)) {
            dxf_line2(dxf, pt1->p.x-pt1->norm.x*20, pt1->p.y-pt1->norm.y*20,  pt1->p.x+pt1->norm.x*20,  pt1->p.y+pt1->norm.y*20);
            dxf_line2(dxf,  pt2->p.x-pt2->norm.x*20, pt2->p.y-pt2->norm.y*20, pt2->p.x+pt2->norm.x*20, pt2->p.y+pt2->norm.y*20);
        }
        dxf_line3(dxf, pt1->p.x - pt1->norm.x*TRK_WIDTH, pt1->p.y - pt1->norm.y*TRK_WIDTH,
                       pt2->p.x - pt1->norm.x*TRK_WIDTH, pt2->p.y- pt1->norm.y*TRK_WIDTH);
        dxf_line3(dxf, pt1->p.x + pt1->norm.x*TRK_WIDTH, pt1->p.y + pt1->norm.y*TRK_WIDTH,
                       pt2->p.x + pt1->norm.x*TRK_WIDTH, pt2->p.y + pt1->norm.y*TRK_WIDTH);

        
    }
    return 0;
}

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
    dxf = dxf_init([s UTF8String]);
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
    

    points = [NSMutableDictionary dictionaryWithCapacity:100];
    turnouts = [[NSMutableDictionary alloc]initWithCapacity:10];

    NSXMLNode *le = [ea firstObject];
    NSArray *ptxml = [le nodesForXPath:@"positionablepoint" error:nil];
    for (NSXMLElement *e in ptxml) {
        NSString *ident = [[e  attributeForName:@"ident"] stringValue];
        double x = [[[e  attributeForName:@"x"]objectValue]doubleValue];
        double y = [[[e  attributeForName:@"y"]objectValue]doubleValue];
        //NSLog(@"point %@", ident);
        JMRIpoint *pt = [JMRIpoint pointWithX:x Y:y];
        [points setObject:pt forKey:ident];
    }
    NSMutableArray *tracksegments = [[NSMutableArray alloc]initWithCapacity:100];
    NSArray *toxml = [le nodesForXPath:@"layoutturnout" error:nil];
    for (NSXMLElement *e in toxml) {
        NSString *ident = [[e  attributeForName:@"ident"] stringValue];
        [turnouts setObject:@1 forKey:ident];
        JMRIpoint *pta; JMRIpoint *ptb;
        for (char c = 'a'; c<='d'; c++) {
            double x = [[[e  attributeForName:[NSString stringWithFormat:@"x%c", c]]objectValue]doubleValue];
            double y = [[[e  attributeForName:[NSString stringWithFormat:@"y%c", c]]objectValue]doubleValue];
            JMRIpoint *pt = [JMRIpoint pointWithX:x Y:y];
            [points setObject:pt forKey:[NSString stringWithFormat:@"%@_%c", ident, c]];
            if (c=='a') pta = pt;
            else if (c=='b') ptb = pt;
        }
        get_normal1_for_seg(pta, ptb);
        get_normal1_for_seg(ptb, pta);
        //dxf_line2(dxf, pta->p.x, pta->p.y, ptb->p.x, ptb->p.y);
        
        NSXMLElement *tt = [[NSXMLElement alloc]initWithName:@"tracksegment"];
        [tt addAttribute:[NSXMLNode attributeWithName:@"connect1name" stringValue:[NSString stringWithFormat:@"%@_b", ident]]];
        [tt addAttribute:[NSXMLNode attributeWithName:@"connect2name" stringValue:[NSString stringWithFormat:@"%@_a", ident]]];
        [tt addAttribute:[NSXMLNode attributeWithName:@"ident" stringValue:[NSString stringWithFormat:@"%@_stra", ident]]];
        //NSLog(@"point %@", ident);
        [tracksegments addObject:tt];
        
        double x = [[[e  attributeForName:@"xcen"]objectValue]doubleValue];
        double y = [[[e  attributeForName:@"ycen"]objectValue]doubleValue];
        if ((1)) {
            JMRIpoint *pt = [JMRIpoint pointWithX:x Y:y];
            [points setObject:pt forKey:[NSString stringWithFormat:@"%@_m", ident]];
        }
        tt = [[NSXMLElement alloc]initWithName:@"tracksegment"];
        [tt addAttribute:[NSXMLNode attributeWithName:@"arc" stringValue:@"1"]];
        [tt addAttribute:[NSXMLNode attributeWithName:@"connect1name" stringValue:[NSString stringWithFormat:@"%@_c", ident]]];
        [tt addAttribute:[NSXMLNode attributeWithName:@"connect2name" stringValue:[NSString stringWithFormat:@"%@_a", ident]]];
        [tt addAttribute:[NSXMLNode attributeWithName:@"ident" stringValue:[NSString stringWithFormat:@"%@_curv", ident]]];
        //NSLog(@"point %@", ident);
        [tracksegments addObject:tt];
    }

    NSArray *tsxml_ = [le nodesForXPath:@"tracksegment" error:nil];
    [tracksegments addObjectsFromArray:tsxml_];
    for (NSXMLElement *e in tracksegments) {
        trackSegment(e, 1, 0);
    }
    for (;;) {
        int m = 0;
        for (NSXMLElement *e in tracksegments) {
           m |= trackSegment(e, 0, 1);
        }
        if (!m) break;
    }
    if ((0)) {
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
            //fn = "/Users/danielbraun/Documents/H5.xml";
            fn = "/Users/danielbraun/Documents/cheminée7.xml";
            //fn = "/Users/danielbraun/Documents/test.xml";
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

#define LAYER1 "Track"
#define LAYER2 "Track"
#define LAYER3 "TBed"
#define COLOR  2
#define COLOR2 3
#define COLOR3 4

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

static void dxf_line_(void *h, int color, const char *layer, double x1, double y1, double x2, double y2)
{
    FILE *F = (FILE *)h;
    DXF_Point(F, layer, color, x1, YSIGN*y1, 0);
    DXF_Point(F, layer, color, x2, YSIGN*y2, 0);
    DXF_Line(F, layer, color, x1, YSIGN*y1, 0, x2, YSIGN*y2, 0);
}

static void dxf_line(void *h, double x1, double y1, double x2, double y2)
{
    dxf_line_(h, COLOR, LAYER1, x1, y1, x2, y2);
}

static void dxf_line2(void *h, double x1, double y1, double x2, double y2)
{
    dxf_line_(h, COLOR2, LAYER2, x1, y1, x2, y2);
}
static void dxf_line3(void *h, double x1, double y1, double x2, double y2)
{
    dxf_line_(h, COLOR3, LAYER3, x1, y1, x2, y2);
}

static void dxf_arc_(void *h, int color, const char *layer, double x1, double y1, double r, double ang0, double ang1)
{
    FILE *F = (FILE *)h;
    DXF_Point(F, layer, color, x1, YSIGN*y1, 0);
    //DXF_Point(F, LAYER, color, x2, YSIGN*y2, 0);
    //DXF_Line(F, LAYER, color, x1, YSIGN*y1, 0, x2, YSIGN*y2, 0);
    DXF_Arc(F, layer, color, x1, YSIGN*y1, 0, r, ang0, ang1);
}
static void dxf_arc(void *h, double x1, double y1, double r, double ang0, double ang1)
{
    dxf_arc_(h, COLOR, LAYER1, x1, y1, r, ang0, ang1);
}
static void dxf_arc2(void *h, double x1, double y1, double r, double ang0, double ang1)
{
    dxf_arc_(h, COLOR2, LAYER2, x1, y1, r, ang0, ang1);
}
static void dxf_arc3(void *h, double x1, double y1, double r, double ang0, double ang1)
{
    dxf_arc_(h, COLOR3, LAYER3, x1, y1, r, ang0, ang1);
}

#pragma mark -

#define SQR(_x) ((_x)*(_x))

static void find_circle2(JMRIpoint *pt1, JMRIpoint *pt2, vector_t *center, double *radius, double *ang1, double *ang2)
{
    double x1 = pt1->p.x; double x2 = pt2->p.x; double y1=pt1->p.y; double y2 = pt2->p.y;

    double a,b,c,d;
    double cx = 0.0, cy;
    int cxd = 0;
    if (!pt1->ndone) abort();
    if (fabs(pt1->norm.x)<0.00001) {
        NSLog(@"hou1");
        a=0;
        cx = pt1->p.x;
        cxd = 1;
    } else {
        a = pt1->norm.y/pt1->norm.x;
    }
    // d = y1-cx1
    b = y1-a*x1;
    if (pt2->ndone) {
        if (!pt2->norm.x) {
            NSLog(@"hou2");
            if (cxd) {
                NSLog(@"big problem here");
                //abort();
            }
            cxd=2;
            cx = pt2->p.x;
            c=0;
        } else {
            c = pt2->norm.y/pt2->norm.x;
        }
        // d = y1-cx1
        d = y2-c*x2;
    } else {
        // médiatrice
        double xm = x1 + (x2 - x1)/2;
        double ym = y1 + (y2 - y1)/2;
        if ((1)) {
            DXF_Point((FILE *)dxf, "Layer", 2, xm, -ym, 0);
        }
        if (y2-y1==0) {
            NSLog(@"hou3");
            if (cxd) abort();
            cx = xm;
            cxd = 3;
            c = 0;
        } else {
            c = -(x2 - x1)/(y2 - y1);
        }
        d = ym - c*xm;
        if  ((0)) dxf_line(dxf, (xm-100),(xm-100)*c+d/*y1*/, (xm+100), (xm+100)*c+d);
    }
    
    if  ((0)) dxf_line2(dxf, (x1-100),(x1-100)*a+b/*y1*/, (x1+100), (x1+100)*a+b);
    if  ((0)) dxf_line2(dxf, (x2-100),(x2-100)*c+d/*y2*/, (x2+100), (x2+100)*c+d);
    switch (cxd) {
    case 0: // normal case
        cx = (d-b)/(a-c);
        cy = a*cx+b;
        double e = fabs((c*cx+d) - cy);
        if (e>0.01) abort();
        break;
    case 1: // cx = x1
        cy = c*cx+d;
        break;
    case 2: // cx = x2 or xm
    case 3:
        cy = a*cx+b;
        break;
    default:
        abort();
        break;
    }
    center->x = cx; center->y=cy;

    

    // le cercle passe par pt1 et pt2
    // (cx-x1)^2 + (cy -x1)^2 = r^2
    // (cx-x2)^2 + (cy -y2)^2 = r^2
    double r1 = sqrt(SQR(cx - x1)+SQR(cy - y1));
    double r2 = sqrt(SQR(cx - x2)+SQR(cy - y2));
    double r = r1;
    *radius = r;
#if 0
#if 0
    double t1 = (y1-cy)/(x1-cx);
    double a1 = atan(t1)*360/(2*M_PI);
    if (x1-cx < 0) a1 = -a1;
    double t2 = (y2-cy)/(x2-cx);
    double a2 = atan(t2)*360/(2*M_PI);
    if (x2-cx<0) a2 = -a2;
#else
    double cosa1 = (x1-cx)/r;
    double sina1 = (y1-cy)/r;
    double cosa2 = (x2-cx)/r;
    double sina2 = (y2-cy)/r;
    double a1 = acos(cosa1) * 180 / M_PI;
    double a1b = asin(sina1) * 180 / M_PI;
    double a2 = acos(cosa2) * 180 / M_PI;
    double a2b = asin(sina2) * 180 / M_PI;
#endif
#endif
    double a1 = -atan2(y1-cy, x1-cx)* 180 / M_PI;
    double a2 = -atan2(y2-cy, x2-cx)* 180 / M_PI;
    if (isnan(a1) || isnan(a2)) {
        NSLog(@"nan");
    }
    if (a1<a2) {
        *ang1 = a1;
        *ang2 = a2;
    } else {
        *ang1 = a2;
        *ang2 = a1;
    }
    if ((*ang1<-90) && (*ang2>90)) {
        double t = *ang1 + 360;
        *ang1 = *ang2;
        *ang2 = t;
    }
    /*
    *ang1 = -a2;
    *ang2 = -a1;
     */
    if ((1)) {
        double da = fabs(*ang1-*ang2);
        NSLog(@"** angle %f %f (%f)\n", *ang1, *ang2, da);
        double e1 = fabs(da-13);
        double e2 = fabs(da-30);
        double e3 = fabs(da-45);
        if ((e1>1) && (e2>1) && (e3>1)) {
            NSLog(@"** angle strange r=%f\n", r);
        }
    }
    if (!pt2->ndone) {
        pt2->ndone = 1;
        pt2->norm = vect_normalize(vect_sub(pt2->p, *center));
        if (!pt2->norm.x && !pt2->norm.y) abort();
    }
    if ((1)) {
        dxf_line2(dxf, x1-pt1->norm.x*20, y1-pt1->norm.y*20, x1+pt1->norm.x*20, y1+pt1->norm.y*20);
        dxf_line2(dxf, x2-pt2->norm.x*20, y2-pt2->norm.y*20, x2+pt2->norm.x*20, y2+pt2->norm.y*20);
    }
}

static void find_circle1(JMRIpoint *pt1, JMRIpoint *pt2, vector_t *center, double *radius, double *ang1, double *ang2)
{
    abort();
    /* mediatrice du segment pt1 pt2 */
    double x1 = pt1->p.x; double x2 = pt2->p.x; double y1=pt1->p.y; double y2 = pt2->p.y;

    if (fabs(pt1->p.y - pt2->p.y)>0.01) {
        double xm = x1 + (x2 - x1)/2;
        double ym = y1 + (y2 - y1)/2;
        if ((0)) {
            DXF_Point((FILE *)dxf, "Layer", 2, xm, -ym, 0);
        }
        double a = -(x2 - x1)/(y2 - y1);
        double b = ym - a*xm;
        // normale en pt1
        // y = c x + d
        // y = (n.y/n.x) x + d
        double c = pt1->norm.y/pt1->norm.x;
        // d = y1-cx1
        double d = y1-c*x1;
        if  ((0)) dxf_line(dxf, xm, ym, xm+100, a*(xm+100)+b);
        if  ((0)) dxf_line(dxf, pt1->p.x, pt1->p.y, (pt1->p.x+100), (pt1->p.x+100)*c+d);
        // intersection
        // ax+b = cx+d, (a-c)x=(d-b), x=(d-b)/(a-c)
        double cx = (d-b)/(a-c);
        double cy = a*cx+b;
        double e = fabs((c*cx+d) - cy);
        if (e>0.01) abort();
        // le cercle passe par pt1 et pt2
        // (cx-x1)^2 + (cy -x1)^2 = r^2
        // (cx-x2)^2 + (cy -y2)^2 = r^2
        double r1 = sqrt(SQR(cx - x1)+SQR(cy - y1));
        double r2 = sqrt(SQR(cx - x2)+SQR(cy - y2));
        double r = r1;
        
#if 0
        // (1) cx^2 - 2*cx*x1 + x1^2  + cy^2-2*cx+y1^2 = r
        // (2) cx^2 - 2*cx*x2 + x2^2  + cy^2-2*cx+y2^2 = r
        //
        /*
         sagemath code:
         x1 = var('x1'); x2 = var('x2'); y1=var('y1'); y2=var('y2'); xm=var('xm'); ym=var('ym'); cx=var('cx');cy=var('cy'); r =var('r')
         a =  -(x1 - x2)/(y1 - y2)
         b = ym - a*xm
         solve([r^2==(cx-x1)^2 + (cy -x1)^2, r^2==(cx-x2)^2 + (cy -x2)^2, cy == a*cx+b], r, cx, cy)
         
         
         [[r == sqrt(x1^4 - 2*x1^3*x2 + 2*x1^2*x2^2 - 2*x1*x2^3 + x2^4 + 2*(x1^2 - 2*x1*x2 + x2^2)*xm^2 + (x1^2 + x2^2)*y1^2 + (x1^2 + x2^2)*y2^2 + 2*(y1^2 - 2*y1*y2 + y2^2)*ym^2 - 2*(x1^3 - x1^2*x2 - x1*x2^2 + x2^3)*xm + 2*(2*x1^2*x2 - 2*x1*x2^2 - (x1^2 - x2^2)*xm)*y1 - 2*(2*x1^2*x2 - 2*x1*x2^2 - (x1^2 - x2^2)*xm + (x1^2 + x2^2)*y1)*y2 - 2*((x1 + x2)*y1^2 + (x1 + x2)*y2^2 + (x1^2 - x2^2 - 2*(x1 - x2)*xm)*y1 - (x1^2 - x2^2 - 2*(x1 - x2)*xm + 2*(x1 + x2)*y1)*y2)*ym)/(x1 - x2 - y1 + y2), cx == ((x1 - x2)*xm - (x1 + x2)*y1 + (x1 + x2)*y2 + (y1 - y2)*ym)/(x1 - x2 - y1 + y2), cy == (x1^2 - x2^2 - (x1 - x2)*xm - (y1 - y2)*ym)/(x1 - x2 - y1 + y2)],
         [r == -sqrt(x1^4 - 2*x1^3*x2 + 2*x1^2*x2^2 - 2*x1*x2^3 + x2^4 + 2*(x1^2 - 2*x1*x2 + x2^2)*xm^2 + (x1^2 + x2^2)*y1^2 + (x1^2 + x2^2)*y2^2 + 2*(y1^2 - 2*y1*y2 + y2^2)*ym^2 - 2*(x1^3 - x1^2*x2 - x1*x2^2 + x2^3)*xm + 2*(2*x1^2*x2 - 2*x1*x2^2 - (x1^2 - x2^2)*xm)*y1 - 2*(2*x1^2*x2 - 2*x1*x2^2 - (x1^2 - x2^2)*xm + (x1^2 + x2^2)*y1)*y2 - 2*((x1 + x2)*y1^2 + (x1 + x2)*y2^2 + (x1^2 - x2^2 - 2*(x1 - x2)*xm)*y1 - (x1^2 - x2^2 - 2*(x1 - x2)*xm + 2*(x1 + x2)*y1)*y2)*ym)/(x1 - x2 - y1 + y2),
         cx == ((x1 - x2)*xm - (x1 + x2)*y1 + (x1 + x2)*y2 + (y1 - y2)*ym)/(x1 - x2 - y1 + y2),
         cy == (x1^2 - x2^2 - (x1 - x2)*xm - (y1 - y2)*ym)/(x1 - x2 - y1 + y2)]]
         */
        double x1 = pt1->p.x; double x2 = pt2->p.x; double y1=pt1->p.y; double y2 = pt2->p.y;
        double cx = ((x1 - x2)*xm - (x1 + x2)*y1 + (x1 + x2)*y2 + (y1 - y2)*ym)/(x1 - x2 - y1 + y2);
        double cy = a*cx+b;
        double r = sqrt(SQR(cx-x1) + SQR(cy -y1));
        //NSLog(@"cx,cy : %f %f   r=%f\n", cx , cy, r);
        center->x = cx; center->y = cy;
#endif
        *radius = r;
        double t1 = (y1-cy)/(x1-cx);
        double a1 = atan(t1)*360/(2*M_PI);
        *ang1 = 0;//a1;
        double t2 = (y2-cy)/(x2-cx);
        double a2 = atan(t2)*360/(2*M_PI);
        *ang2 = 300;//a2;
    } else {
        NSLog(@"z");

    }
    NSLog(@"hop");
}
