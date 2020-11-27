//
//  TesItems.m
//  TrainLayout
//
//  Created by Daniel BRAUN on 12/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "LayoutItem.h"
#import "LayoutRect.h"
#import "TcoLayout.h"

@interface TesItems : XCTestCase

@end

@interface TEndpoint : NSObject {
    @public
    int x;
    int y;
}
@end
@implementation TEndpoint


@end



@interface EndpointDict : NSObject <EndPointDict> {
    NSMutableDictionary *endpointDict;
}
@end

@implementation EndpointDict


- (void) sedEndPoint:(NSString *)name X:(int)x Y:(int)y
{
    if (!endpointDict) endpointDict = [[NSMutableDictionary alloc]initWithCapacity:10];
    TEndpoint *e = [[TEndpoint alloc]init];
    e->x = x; e->y = y;
    [endpointDict setObject:e forKey:name];
}

- (TEndpoint *) _getEndPoint:(NSString *)name
{
    TEndpoint *r;
    if (endpointDict) {
        r = [endpointDict objectForKey:name];
        if (r) return r;
    }
    return nil;
}
- (BOOL) getEndPoint:(NSString *)name rX:(int *)x rY:(int *)y
{
    TEndpoint *e = [self _getEndPoint:name];
    if (!e) return NO;
    *x = e->x;
    *y = e->y;
    return YES;
}


@end


@implementation TesItems {
    EndpointDict *dic;
    LayoutItem *item;
}

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    dic = [[EndpointDict alloc]init];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

    
- (void) setItemDir:(char)dir
{
    NSString *s = [NSString stringWithFormat:@"8%c", dir];
    item = [[LayoutItem alloc]initWithNum:1 name:@"t1" leftEnd:@[@"A", @"L"] rightEnd:@[@"B", @"R"] path:s];
}

- (BOOL) testOccupied:(LayoutRect *)r t:(int)maxt xfn:(int(^)(int))xfn yfn:(int(^)(int))yfn
{
    for (int t=0; t<maxt; t++) {
        int x,y;
        x = xfn(t);
        y = yfn(t);
        BOOL occ = [r containsX:x Y:y];
        if (!occ) return NO;
    }
    return YES;
}

- (void) test1
{
    [self setItemDir:'-'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='-');
    XCTAssert([rect begx]==0);
    XCTAssert([rect begy]==0);
    XCTAssert([rect endx]==8);
    XCTAssert([rect endy]==0);
    XCTAssert([self testOccupied:rect t:8 xfn:^(int t) { return t;} yfn:^(int t) { return 0;}]);
}

- (void) test2
{
    [self setItemDir:'|'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='|');
    XCTAssert([rect begx]==0);
    XCTAssert([rect begy]==0);
    XCTAssert([rect endx]==0);
    XCTAssert([rect endy]==8);
    XCTAssert([self testOccupied:rect t:8 xfn:^(int t) { return 0;} yfn:^(int t) { return t;}]);
}

- (void) test3
{
    [self setItemDir:'/'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='/');
    XCTAssert([rect begx]==0);
    XCTAssert([rect begy]==0);
    XCTAssert([rect endx]==8);
    XCTAssert([rect endy]==8);
    XCTAssert([self testOccupied:rect t:8 xfn:^(int t) { return t;} yfn:^(int t) { return t;}]);
}



- (void) test4
{
    [self setItemDir:'\\'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='\\');
    XCTAssert([rect begx]==0);
    XCTAssert([rect begy]==0);
    XCTAssert([rect endx]==8);
    XCTAssert([rect endy]==-8);
    XCTAssert([self testOccupied:rect t:8 xfn:^(int t) { return t;} yfn:^(int t) { return -t;}]);
}

- (BOOL) checkEndPoint:(NSString *)name x:(int)ex y:(int)ey
{
    int x,y;
    [dic getEndPoint:name rX:&x rY:&y];
    if (x!=ex) return NO;
    if (y!=ey) return NO;
    return YES;
}
- (void) testL1
{
    [dic sedEndPoint:@"A" X:42 Y:23];
    [self setItemDir:'-'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='-');
    XCTAssert([rect begx]==42);
    XCTAssert([rect begy]==23);
    XCTAssert([rect endx]==42+8);
    XCTAssert([rect endy]==23);
    XCTAssert([self checkEndPoint:@"L" x:42 y:23]);
    XCTAssert([self checkEndPoint:@"B" x:42+8 y:23]);
    XCTAssert([self checkEndPoint:@"R" x:42+8 y:23]);
}

- (void) testL2
{
    [dic sedEndPoint:@"A" X:42 Y:23];
    [self setItemDir:'|'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='|');
    XCTAssert([rect begx]==42);
    XCTAssert([rect begy]==23);
    XCTAssert([rect endx]==42);
     XCTAssert([rect endy]==23+8);
    XCTAssert([self checkEndPoint:@"L" x:42 y:23]);
    XCTAssert([self checkEndPoint:@"B" x:42 y:23+8]);
    XCTAssert([self checkEndPoint:@"R" x:42 y:23+8]);
}


- (void) testR1
{
    [dic sedEndPoint:@"B" X:42 Y:23];
    [self setItemDir:'-'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='-');
    XCTAssert([rect begx]==42-8);
    XCTAssert([rect begy]==23);
    XCTAssert([rect endx]==42);
    XCTAssert([rect endy]==23);
    XCTAssert([self checkEndPoint:@"R" x:42 y:23]);
    XCTAssert([self checkEndPoint:@"A" x:42-8 y:23]);
    XCTAssert([self checkEndPoint:@"L" x:42-8 y:23]);
}



- (void) testR2
{
    [dic sedEndPoint:@"B" X:42 Y:23];
    [self setItemDir:'|'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='|');
    XCTAssert([rect begx]==42);
    XCTAssert([rect begy]==23-8);
    XCTAssert([rect endx]==42);
     XCTAssert([rect endy]==23);
    XCTAssert([self checkEndPoint:@"R" x:42 y:23]);
      XCTAssert([self checkEndPoint:@"A" x:42 y:23-8]);
      XCTAssert([self checkEndPoint:@"L" x:42 y:23-8]);
}



- (void) testR3
{
    [dic sedEndPoint:@"B" X:42 Y:23];
    [self setItemDir:'/'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='/');
    XCTAssert([rect begx]==42);
    XCTAssert([rect begy]==23-8);
    XCTAssert([rect endx]==42-8);
     XCTAssert([rect endy]==23);
    XCTAssert([self checkEndPoint:@"R" x:42 y:23]);
      XCTAssert([self checkEndPoint:@"A" x:42-8 y:23-8]);
      XCTAssert([self checkEndPoint:@"L" x:42-8 y:23-8]);
}



- (void) testR4
{
    [dic sedEndPoint:@"B" X:42 Y:23];
    [self setItemDir:'\\'];
    NSArray *r = [item layoutsForVariant:0 endpointDict:dic];
    NSLog(@"r=%@", r);
    XCTAssert([r count]==1);
    LayoutRect *rect = [r firstObject];
    XCTAssert([rect dir]=='\\');
    XCTAssert([rect begx]==42);
    XCTAssert([rect begy]==23+8);
    XCTAssert([rect endx]==42-8);
     XCTAssert([rect endy]==23);
    XCTAssert([self checkEndPoint:@"R" x:42 y:23]);
      XCTAssert([self checkEndPoint:@"A" x:42-8 y:23+8]);
      XCTAssert([self checkEndPoint:@"L" x:42-8 y:23+8]);
}
@end
