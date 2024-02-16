//
//  AppDelegate.h
//  SvgGen
//
//  Created by Daniel BRAUN on 06/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface AppDelegate : NSObject <NSApplicationDelegate, WKUIDelegate, WKNavigationDelegate>

@property (nonatomic, weak) IBOutlet WKWebView *ctoWebView;

- (IBAction) generateAndDisplaySvg:(id)sender;
- (IBAction) saveToFile:(id)sender;


@end

