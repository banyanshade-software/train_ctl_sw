//
//  LayoutRect.h
//  TrainLayout
//
//  Created by Daniel BRAUN on 10/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LayoutRect : NSObject

- (instancetype) initWithDir:(char)_d X:(int)_x Y:(int)_y len:(int)l;
- (instancetype) initWithDir:(char)_d rX:(int)_x rY:(int)_y len:(int)l;

- (void) enumeratePosition:(void (^)(int x, int y, BOOL *stp))blk;
- (BOOL) containsX:(int)x Y:(int)y;

- (char) dir;
- (int) begx;
- (int) begy;
- (int) endx;
- (int) endy;

@end

NS_ASSUME_NONNULL_END
