//
//  LayoutItem.m
//  TrainLayout
//
//  Created by Daniel BRAUN on 07/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "LayoutItem.h"
#import "LayoutRect.h"


@interface PathItem : NSObject {
    @public
    char dir;
    int len;
}

- (instancetype) initWithDir:(char)d len:(int)l;
@end

@implementation PathItem

- (instancetype) initWithDir:(char)d len:(int)l
{
    self = [super init];
    if (!self) return nil;
    dir = d;
    len = l;
    return self;
}
@end
#pragma mark -

@implementation LayoutItem {
    int _num;
    NSString *_name;
    NSArray *left;
    NSArray *right;
    id path;
}
@synthesize num = _num;
@synthesize name = _name;

- (instancetype) initWithNum:(int)num name:(NSString *)name leftEnd:(NSArray *)consLeft
rightEnd:(NSArray *)consRight path:(NSString *)p
{
    self = [super init];
    if (!self) return nil;
    _num = num;
    _name = name;
    left = consLeft;
    right = consRight;
    path = p;
    return self;
}

- (NSArray *) layoutsForVariant:(int)v endpointDict:(NSObject <EndPointDict> *)dic
{
    // find endpoint if any
    BOOL fromRight = NO;
    BOOL fromLeft = NO;
    int x0=0;
    int y0=0;
    for (NSString *en in left) {
        int x,y;
        BOOL f = [dic getEndPoint:en rX:&x rY:&y];
        if (f) {
            if (!fromLeft) {
                fromLeft = YES;
                x0=x;
                y0=y;
            } else {
                if ((x != x0) || (y!=y0)) {
                    return nil;
                }
            }
            break;
        }
    }
    if (!fromLeft) {
        for (NSString *en in right) {
               int x,y;
               BOOL f = [dic getEndPoint:en rX:&x rY:&y];
               if (f) {
                   if (!fromRight) {
                       fromRight = YES;
                       x0=x;
                       y0=y;
                   } else {
                       if ((x != x0) || (y!=y0)) {
                           return nil;
                       }
                   }
                   break;
               }
           }
    }
    if (!fromLeft && !fromRight) {
        // new endpoint
        x0 = 0; y0 = 0;
        if ([left count]) {
            NSString *s = [left firstObject];
            [dic sedEndPoint:s X:0 Y:0];
            fromLeft = YES;
        } else if ([right count]) {
            NSString *s = [right firstObject];
            [dic sedEndPoint:s X:0 Y:0];
            fromRight = YES;
        } else {
            return nil;
        }
    }
    int lx; int ly;
    NSArray *r = [self _layoutsForVariant:v x0:x0 y0:y0 lastx:&lx lasty:&ly fromRight:fromRight];
    if (![r count]) return nil;
    if (fromLeft) {
        for (NSString *en in right) {
            int x,y;
            BOOL f = [dic getEndPoint:en rX:&x rY:&y];
            if (f) {
                if ((x != lx) || (y != ly)) {
                    return nil;
                }
            }
            [dic sedEndPoint:en X:lx Y:ly];
        }
    }
    if (fromRight) {
        for (NSString *en in left) {
            int x,y;
            BOOL f = [dic getEndPoint:en rX:&x rY:&y];
            if (f) {
                if ((x != lx) || (y != ly)) {
                    return nil;
                }
            }
            [dic sedEndPoint:en X:lx Y:ly];
        }
    }
    return r;
}

- (NSArray *) _layoutsForVariant:(int)v x0:(int)x0 y0:(int)y0 lastx:(int *)lastx lasty:(int *)lasty fromRight:(BOOL)fromRight
{
    if ([path isKindOfClass:[NSString class]]) {
        [self parsePath];
    }
    NSArray *pth = (NSArray *)path;
    int x = x0;
    int y = y0;
    NSMutableArray *res = [NSMutableArray arrayWithCapacity:[pth count]];
    NSEnumerator *en = fromRight ? [pth reverseObjectEnumerator] : [pth objectEnumerator];
    for (;;) {
        PathItem *pi = [en nextObject];
        if (!pi) break;
        LayoutRect *r = fromRight ? [[LayoutRect alloc]initWithDir:pi->dir rX:x rY:y len:pi->len] : [[LayoutRect alloc]initWithDir:pi->dir X:x Y:y len:pi->len];
        [res addObject:r];
        x = [r endx];
        y = [r endy];
    }
    *lastx = x;
    *lasty = y;
    return res;
}


- (void) parsePath
{
    if (![path isKindOfClass:[NSString class]]) return;
    // path in form <digit><dir>... with dir=-|/...
    const char *s = [(NSString *)path cStringUsingEncoding:NSUTF8StringEncoding];
    int n = 0;
    NSMutableArray *a = [NSMutableArray arrayWithCapacity:10];
    for (; *s; s++) {
        if (*s>='0' && *s<='9') {
            n = n*10+(*s-'0');
            continue;
        }
        if (!n) continue;
        char dir;
        switch (*s) {
            case '/':
            case '\\':
            case '-':
            case '|':
                dir = *s;
                break;
            default:
                continue;
                break;
        }
        PathItem *pi = [[PathItem alloc]initWithDir:dir len:n];
        [a addObject:pi];
        n=0;
    }
    path = a;
}


 @end
