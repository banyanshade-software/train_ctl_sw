//
//  TcoLayout.m
//  TrainLayout
//
//  Created by Daniel BRAUN on 09/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "TcoLayout.h"
#import "LayoutRect.h"
#import "LayoutItem.h"


@interface Endpoint : NSObject {
    @public
    int x;
    int y;
}
@end
@implementation Endpoint
@end


@interface LayoutGrid : NSObject <EndPointDict>
- (LayoutGrid *) push;
- (BOOL) addLayoutRect:(LayoutRect *)r;
- (void) getMinX:(int *)pminx minY:(int*)pminy maxX:(int *)pmaxX maxY:(int *)pmaxY;
@end

@implementation LayoutGrid {
    LayoutGrid *parent;
    NSMutableArray *rects;
    BOOL mutable;
    NSMutableDictionary *endpointDict;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        mutable = YES;
    }
    return self;
}
-(LayoutGrid *)push
{
    mutable = NO;
    LayoutGrid *ngr = [[LayoutGrid alloc]init];
    if (!ngr) return nil;
    ngr->parent = self;
    ngr->mutable = YES;
    return ngr;
}

- (BOOL) isOccupiedX:(int)x Y:(int)y
{
    for (LayoutRect *r in rects) {
        if ([r containsX:x Y:y]) return YES;
    }
    if (!parent) return NO;
    return [parent isOccupiedX:x Y:y];
}

- (char) occupiedCharX:(int)x Y:(int)y
{
    for (LayoutRect *r in rects) {
        if ([r containsX:x Y:y]) return r.dir;
    }
    if (!parent) return 0;
    return [parent occupiedCharX:x Y:y];
}
- (void) getMinX:(int *)pminx minY:(int*)pminy maxX:(int *)pmaxx maxY:(int *)pmaxy
{
    for (LayoutRect *r in rects) {
        int x = [r begx];
        if (x>*pmaxx) *pmaxx = x;
        if (x<*pminx) *pminx = x;
        x = [r endx];
        if (x>*pmaxx) *pmaxx = x;
        if (x<*pminx) *pminx = x;
        int y = [r begy];
        if (y>*pmaxy) *pmaxy = y;
        if (y<*pminy) *pminy = y;
        y = [r endy];
        if (y>*pmaxy) *pmaxy = y;
        if (y<*pminy) *pminy = y;
    }
    [parent getMinX:pminx minY:pminy maxX:pmaxx maxY:pmaxy];
}

- (BOOL) addLayoutRect:(LayoutRect *)r
{
    if (!mutable) {
        return NO;
    }
    if (!rects) rects = [[NSMutableArray alloc]initWithCapacity:5];
    // check if rect is free
    BOOL __block occ = NO;
    [r enumeratePosition:^(int x, int y, BOOL *stp) {
        if ([self isOccupiedX:x Y:y]) {
            *stp = YES;
            occ = YES;
        }
    }];

    // add it
    [rects addObject:r];
    return YES;
}




- (void) sedEndPoint:(NSString *)name X:(int)x Y:(int)y
{
    if (!endpointDict) endpointDict = [[NSMutableDictionary alloc]initWithCapacity:10];
    Endpoint *e = [[Endpoint alloc]init];
    e->x = x; e->y = y;
    [endpointDict setObject:e forKey:name];
}

- (Endpoint *) _getEndPoint:(NSString *)name
{
    Endpoint *r;
    if (endpointDict) {
        r = [endpointDict objectForKey:name];
        if (r) return r;
    }
    return [parent _getEndPoint:name];
}
- (BOOL) getEndPoint:(NSString *)name rX:(int *)x rY:(int *)y
{
    Endpoint *e = [self _getEndPoint:name];
    if (!e) return NO;
    *x = e->x;
    *y = e->y;
    return YES;
}


@end

#pragma mark -

@implementation TcoLayout {
    NSMutableDictionary *dico;
    LayoutGrid *grid;
    int minx; int maxx; int miny; int maxy;
}


- (BOOL) layout:(NSArray *)a
{
    dico = [[NSMutableDictionary alloc]initWithCapacity:[a count]];
    for (LayoutItem *it in a) {
        [dico setObject:it forKey:it.name];
    }
    
    LayoutGrid *gr = [[LayoutGrid alloc]init];
    grid = [self _layoutItems:a with:gr];
    if (!grid) {
        minx = 0; miny = 0; maxx =0; maxy =0;
        return NO;
    }
    [self getminmax];
    return grid ? YES : NO;
}

- (void) getminmax
{
    miny = 99999; minx = 99999; maxx = -99999; maxy = -99999;
    [grid getMinX:&minx minY:&miny maxX:&maxx maxY:&maxy];
    NSLog(@"X:[%d-%d] Y:[%d:%d]", minx,maxx, miny, maxy);
    NSMutableString *s = [[NSMutableString alloc]init];
    for (int l=miny; l<maxy; l++) {
        for (int c=minx; c<maxx; c++) {
            char chr = [grid occupiedCharX:c Y:l];
            if (!chr) chr = ' ';
            [s appendFormat:@"%c", chr];
        }
        [s appendString:@"\n"];
    }
    NSLog(@"dump :\n%@", s);
}
- (LayoutGrid *) _layoutItems:(NSArray *)items with:(LayoutGrid *)g
{
    if (![items count]) return g;
    LayoutItem *first = [items objectAtIndex:0];
    NSArray *cdr = [items subarrayWithRange:NSMakeRange(1, [items count]-1)];
    for (int v=0; v<8; v++) {
        LayoutGrid *ngr = [self _layoutOne:first with:g variant:v];
        if (!ngr) continue;
        ngr = [self _layoutItems:cdr with:ngr];
        if (ngr) return ngr;
    }
    return nil;
}

- (LayoutGrid *) _layoutOne:(LayoutItem *)it with:(LayoutGrid *)g variant:(int)v
{
    LayoutGrid *ngr = [g push];
    NSArray *lv = [it layoutsForVariant:v endpointDict:ngr];
    if (!lv) return nil;
    for (LayoutRect *r in lv) {
        BOOL ok = [ngr addLayoutRect:r];
        if (!ok) {
            return nil;
        }
    };
    return ngr;
}


@end
