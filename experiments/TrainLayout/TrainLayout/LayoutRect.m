//
//  LayoutRect.m
//  TrainLayout
//
//  Created by Daniel BRAUN on 10/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "LayoutRect.h"

@implementation LayoutRect {
    char direction; // - | / ..
    int x; int y;
    int l;
}

- (instancetype) initWithDir:(char)_d X:(int)_x Y:(int)_y len:(int)len
{
    self = [super init];
    if (!self) return self;
    x = _x;
    y = _y;
    direction = _d;
    l = len;
    return self;
}


- (BOOL) containsX:(int)xx Y:(int)yy
{
    switch (direction) {
        case '|':
            if (xx != x) return NO;
            if ((yy>=y) && (yy<y+l)) return YES;
            return NO;
            break;
        case '-':
            if (yy != y) return NO;
            if ((xx>=x) && (xx<x+l)) return YES;
            return NO;
            break;
        case '/':
            if (xx-yy != x-y) return NO;
            if ((xx>=x) && (xx<=x+l)) return YES;
            return NO;
            break;
        case '\\':
            if (xx+yy != x+y) return NO;
            if ((xx>=x) && (xx<x+l)) return YES;
            return NO;
            break;
        default:
            break;
    }
    return NO;
}

- (void) enumeratePosition:(void (^)(int x, int y, BOOL *stp))blk
{
    BOOL stop = NO;
    switch (direction) {
        case '|':
            for (int i=0; i<l; i++) {
                blk(x,y+i, &stop);
                if (stop) break;
            }
            break;
        case '-':
            for (int i=0; i<l; i++) {
                blk(x+i, y, &stop);
                if (stop) break;
            }
            break;
        case '/':
            for (int i=0; i<l; i++) {
                blk(x+i, y+i, &stop);
                if (stop) break;
            }
            break;
        case '\\':
            for (int i=0; i<l; i++) {
                blk(x+i, y-i, &stop);
                if (stop) break;
            }
            break;
        default:
            break;
    }
}



- (instancetype) initWithDir:(char)_d rX:(int)_x rY:(int)_y len:(int)l
{
    switch (_d) {
        case '-':
            _x = _x-l;
            break;
        case '|':
            _y = _y-l;
            break;
        case '/':
            _x = _x-l;
            _y = _y-l;
            break;
        case '\\':
            _x = _x-l;
            _y = _y+l;
            break;
        default:
            break;
    }
    return [self initWithDir:_d X:_x Y:_y len:l];
}

- (int) endx
{
    switch (direction) {
        default:
            return x+l;
            break;
        case '|':
            return x;
    }
}
- (int) endy
{
    switch (direction) {
        case '-':
            return y;
            break;
        case '\\':
            return y-l;
            break;
        case '/':
            return y+l;
            break;
        default:
            return y+l;
            break;
    }
}
- (int) begx
{
    return x;
}
- (int) begy
{
    return y;
}

- (char) dir
{
    if (direction=='\\') {
        NSLog(@"hop");
    }
    return direction;
}

- (NSString *) description
{
    return [NSString stringWithFormat:@"LRect %c <%d %d %d %d>",
            direction, self.begx, self.begy, self.endx, self.endy];
}
@end
