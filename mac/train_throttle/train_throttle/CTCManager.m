//
//  CTCManager.m
//  train_throttle
//
//  Created by Daniel BRAUN on 04/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import "CTCManager.h"
#import "AppDelegate.h"
#include "topology.h"
#include "occupency.h"

@implementation CTCManager {
    WKUserContentController *wkuserctrl;
}



- (void) loadHtml
{
    NSURL *u = [[NSBundle mainBundle] URLForResource:@"uitrack" withExtension:@"html"];
    NSError *err;
    NSString *ctohtml = [NSString stringWithContentsOfURL:u encoding:NSUTF8StringEncoding error:&err];
    [_ctoWebView loadHTMLString:ctohtml baseURL:nil];
    
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        // hide all info from this blk
        NSString *js = @"Array.from(document.getElementsByClassName('trinfo'), el => el.style.visibility = 'hidden')";
        [self->_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
        js = @"Array.from(document.getElementsByClassName('track'), el => el.style.stroke = 'darkgray')";
        [self->_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
        // add callback for turnouts
        js = @"Array.from(document.getElementsByClassName('tncircle'), el => el.addEventListener(\"click\", function () {\
           window.webkit.messageHandlers.ctc.postMessage(\"c\"+el.getAttribute('id'));} ));";
        [self->_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
    });
    wkuserctrl = _ctoWebView.configuration.userContentController;
    [wkuserctrl addScriptMessageHandler:self name:@"ctc"];
}



static int _dim(int col, int dim)
{
    col = col/(dim+1);
    col = col + 40 * dim;
    if (col>255) col = 255;
    return col;
}


- (NSString *) colorForTrain:(int)train dim:(int)dim
{
    int r,g,b;
    switch (train) {
        case 0: r=60; b=255; g=10; break;
        case 1: r=20; b=200; g=200; break;
        case 2: r=200; b=0; g=200; break;
        case 3: r=200; b=255; g=0; break;
        default: r=50; b=50; g=50; break;
    }
    r = _dim(r, dim);
    g = _dim(g, dim);
    b = _dim(b, dim);
    return [NSString stringWithFormat:@"#%2.2X%2.2X%2.2X", r, g, b ];
}


- (void) uitrac_change_tn:(int)tn val:(int)v
{
    NSString *t1 = [NSString stringWithFormat:@"to%d%c", tn, v ? 't' : 's'];
    NSString *t2 = [NSString stringWithFormat:@"to%d%c", tn, v ? 's' : 't'];
    NSString *js = [NSString stringWithFormat:@"document.getElementById('%@').style['stroke-width'] = 1; document.getElementById('%@').style['stroke-width'] = 8;", t2, t1];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];

}


- (void) uitrac_change_tn_reser:(int)tn train:(int)train
{
    NSString *circle = [NSString stringWithFormat:@"c%d", tn];
    NSString *col = (train>=0) ? [self colorForTrain:train dim:0] : @"darkgray";
    NSString *js = [NSString stringWithFormat:@"el=document.getElementById('%@'); el.style.stroke=\"%@\";  el.style['stroke-width']=\"%dpx\";",
                    circle, col , train==0xFF ? 1 : 2];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
           if (err) {
               NSLog(@"js error : %@\n", err);
           }
       }];
}


- (void) uitrac_change_blk:(int) blk val:(int)v train:(int)trn sblk:(int)sblk
{
    NSString *js;
    //NSString *nblk = [NSString stringWithFormat:@"BLK%d", blk];
    NSString *col = @"white";
    NSString *strn = (trn == 0xFF) ? nil : [NSString stringWithFormat:@"(T%d", trn];
    /*
    switch (v) {
        case BLK_OCC_FREE: // BLK_OCC_FREE:
            col = @"darkgray";
            break;
        case BLK_OCC_STOP:
            col = @"brown";
            strn = [strn stringByAppendingString:@"--)"];
            break;
        case BLK_OCC_LEFT:
            col = @"orange";
            strn = [strn stringByAppendingString:@"<<)"];
            break;
        case BLK_OCC_RIGHT:
            col = @"red";
            strn = [strn stringByAppendingString:@">>)"];
            break;
        case BLK_OCC_C2:
            col = @"yellow";
            strn = [strn stringByAppendingString:@"••)"];
            break;
        default:
            if ((v>=BLK_OCC_DELAY1) && (v<=BLK_OCC_DELAYM)) {
                col = @"salmon";
                strn = [strn stringByAppendingString:@")"];
            }
            break;
    }
     */
    int d;
    if (v==BLK_OCC_FREE) col = @"darkgray";
    else {
        switch (v) {
            case BLK_OCC_STOP:  d=1; strn = [strn stringByAppendingString:@"--)"]; break;
            case BLK_OCC_C2:    d=2; strn = [strn stringByAppendingString:@"..)"]; break;
            case BLK_OCC_LEFT:  d=0; strn = [strn stringByAppendingString:@" <)"]; break;
            case BLK_OCC_RIGHT: d=0; strn = [strn stringByAppendingString:@" >)"]; break;
            default:
                strn = [strn stringByAppendingString:@")"];
                d = 5;
                break;
        }
        col = [self colorForTrain:trn dim:d];
    }
    //js = [NSString stringWithFormat:@"document.getElementById('%@').style.stroke = '%@';", nblk, col];
    js = [NSString stringWithFormat:@"segs = document.getElementsByClassName('CANTON%d');\nArray.from(segs, el => el.style.stroke = '%@');", blk, col];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];
    
    // hide all info from this blk
    js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('trinfo_c%d'), el => el.style.visibility = 'hidden')", blk];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];
    if (sblk==255) {
        NSLog(@"hop");
    }
    if (sblk>=0) {
        js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('trinfo_s%d'), el => el.style.visibility = 'visible'); document.getElementById('tr%d').textContent = '%@';", sblk, sblk,
              strn ? strn : @""];
        [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
        
    } else {
        NSLog(@"no sblk info");
    }
    
    /*
    js = [NSString stringWithFormat:@"document.getElementById('txtc%d').textContent = '%@'; document.getElementById('txtc%d').style.visibility = '%@';",
          blk, strn ? strn : @"",
          blk, strn ? @"visible" : @"hidden"];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];
    */
}

- (void)userContentController:(nonnull WKUserContentController *)userContentController didReceiveScriptMessage:(nonnull WKScriptMessage *)message
{
    NSString *s = message.body;
        if ([s length] < 3) goto badmsg;
        s = [s substringFromIndex:2];
        int n = [s intValue];
        [_appDelegate toggleTurnout:n];
        return;
        
    badmsg:
        NSLog(@"ho");
}

@end
