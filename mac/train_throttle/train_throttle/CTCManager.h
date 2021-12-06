//
//  CTCManager.h
//  train_throttle
//
//  Created by Daniel BRAUN on 04/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//


#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

NS_ASSUME_NONNULL_BEGIN

@class AppDelegate;
@interface CTCManager : NSObject <WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler>


@property (nonatomic, weak) IBOutlet WKWebView *ctoWebView;
@property (nonatomic, weak) IBOutlet AppDelegate *appDelegate;


- (void) loadHtml;

- (void) uitrac_change_tn:(int)tn val:(int)v;

- (void) uitrac_change_tn_reser:(int)tn train:(int)train;

- (void) uitrac_change_blk:(int) blk val:(int)v train:(int)trn sblk:(int)sblk;


// for trackplan2
- (void) uitrac_change_sblk:(int) sblk val:(int)v train:(int)trn;
- (void) hideTrainInfos;

@end

NS_ASSUME_NONNULL_END
