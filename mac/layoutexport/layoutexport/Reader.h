//
//  Reader.h
//  layoutexport
//
//  Created by Daniel BRAUN on 26/05/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface Reader : NSObject

- (BOOL) readLayoutFile:(NSString *)path;

@end

NS_ASSUME_NONNULL_END
