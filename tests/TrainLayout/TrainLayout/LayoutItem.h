//
//  LayoutItem.h
//  TrainLayout
//
//  Created by Daniel BRAUN on 07/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class LayoutRect;

@protocol EndPointDict <NSObject>

- (void) sedEndPoint:(NSString *)name X:(int)x Y:(int)y;
- (BOOL) getEndPoint:(NSString *)name rX:(int *)x rY:(int *)y;

@end


@interface LayoutItem : NSObject
@property (readonly) int num;
@property (readonly,retain) NSString *name;


- (instancetype) initWithNum:(int)num name:(NSString *)name leftEnd:(NSArray *)consLeft
                    rightEnd:(NSArray *)consRight path:(NSString *)p;

- (NSArray *) layoutsForVariant:(int)v endpointDict:(NSObject <EndPointDict> *)dic;

@end

NS_ASSUME_NONNULL_END
