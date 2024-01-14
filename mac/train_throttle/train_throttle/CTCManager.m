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
    int _highlightIna;
    int _highlightCanton;
}



- (instancetype)init
{
    self = [super init];
    if (self) {
        _highlightIna = -1;
        _highlightCanton = -1;
    }
    return self;
}
- (void) loadHtml
{
    NSURL *u = [[NSBundle mainBundle] URLForResource:@"uitrack" withExtension:@"html"];
    NSAssert(u, @"uitrack.html missing");
    NSError *err;
    NSString *ctohtml = [NSString stringWithContentsOfURL:u encoding:NSUTF8StringEncoding error:&err];
    WKNavigation *nv = [_ctoWebView loadHTMLString:ctohtml baseURL:nil];
    NSAssert(nv, @"load failed");
    
    /*dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self installJavascript];
    });*/
    wkuserctrl = _ctoWebView.configuration.userContentController;
    [wkuserctrl addScriptMessageHandler:self name:@"ctc"];
}

- (void) installJavascript
{
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
}
- (void)webView:(WKWebView *)webView didFinishNavigation:(null_unspecified WKNavigation *)navigation
{
    [self installJavascript];
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


- (void) uitrac_change_tn:(int)tn val:(enum topo_turnout_state)v
{
    int t = 0;
    int s = 0;
    switch (v) {
        case topo_tn_straight:
            s = 1; t = 0;
            break;
        case topo_tn_turn:
            s = 0; t = 1;
            break;
        case topo_tn_moving:
            s = 1; t = 1;
            break;
        case topo_tn_undetermined: // FALLTHRU
        default:
            s = 0; t = 0;
            break;
    }
    NSString *ts = [NSString stringWithFormat:@"to%ds", tn];
    NSString *tt = [NSString stringWithFormat:@"to%dt", tn];
    NSString *js = [NSString stringWithFormat:@"document.getElementById('%@').style['stroke-width'] = %d; document.getElementById('%@').style['stroke-width'] = %d;",
                    ts, s ? 8 : 1, tt, t ? 8: 1];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"turnout js error : %@\n", err);
            /*
             normally occurs for future turnouts
             */
        }
    }];

}


- (void) uitrac_change_tn_reser:(int)tn train:(int)train
{
    NSString *circle = [NSString stringWithFormat:@"c%d", tn];
    NSString *col = (train>=0) ? [self colorForTrain:train dim:0] : @"darkgray";
    NSString *js = [NSString stringWithFormat:@"el=document.getElementById('%@'); el.style.stroke=\"%@\";  el.style['stroke-width']=\"%dpx\";",
                    circle, col , train==0xFF ? 1 : 4];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
           if (err) {
               NSLog(@"js error : %@\n", err);
           }
       }];
}


- (void) uitrac_change_blk:(uint8_t) blk val:(uint8_t)v train:(uint8_t)trn sblk:(uint8_t)sblk
{
    NSString *js;
    //NSString *nblk = [NSString stringWithFormat:@"BLK%d", blk];
    NSString *col = @"white";
    NSString *strn = (trn == 0xFF) ? nil : [NSString stringWithFormat:@"(T%d", trn];
   
    
    int d;
    if (v==BLK_OCC_FREE) col = @"darkgray";
    else {
        switch (v) {
            case BLK_OCC_LOCO_STOP: d=1; strn = [strn stringByAppendingString:@"--)"]; break;
            case BLK_OCC_C2:        d=2; strn = [strn stringByAppendingString:@"..)"]; break;
            case BLK_OCC_LOCO_LEFT: d=0; strn = [strn stringByAppendingString:@" <)"]; break;
            case BLK_OCC_LOCO_RIGHT:d=0; strn = [strn stringByAppendingString:@" >)"]; break;
            case BLK_OCC_CARS:  d=0; strn = [strn stringByAppendingString:@"~~)"]; break;
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
}

- (void) uitrac_change_pres:(uint32_t) bitfield
{
    static uint32_t lasvalue = 0;
    for (int i = 0; i<32; i++) {
        int l = bitfield & (1<<i);
        int p = lasvalue & (1<<i);
        if (l==p) continue;
        // <polyline id="SBLK05" class="track CANTON3" stroke="#000000" stroke-width="5px" fill="none" points="600,320 680,320 720,280 720,120"></polyline>
        NSString *js = [NSString stringWithFormat:@"document.getElementById('SBLK%2.2d').style['stroke-width'] = '%s';", i, l ? "10px" : "5px"];
        [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
    }
    lasvalue = bitfield;
}

- (void) uitrac_change_sblk:(int) sblk val:(int)v train:(int)trn
{
    if (trn<0) trn=0xFF;
    if (sblk<0) return;
    NSString *js;
    //NSString *nblk = [NSString stringWithFormat:@"BLK%d", blk];
    NSString *col = @"white";
    NSString *strn = (trn == 0xFF) ? nil : [NSString stringWithFormat:@"   T%d", trn];
   
    
    if (v==BLK_OCC_FREE) col = @"darkgray";
    else {
        col = [self colorForTrain:trn dim:4];
        //col = [col stringByAppendingString:@"55"]; // alpha
    }
    
    
  
    
    if (!v) {
        js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('trinfo_s%d'), el => el.style.visibility = 'hidden');", sblk];
        [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
    } else {
        js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('trinfo_s%d'), el => el.style.visibility = 'visible');  Array.from(document.getElementsByClassName('rectinfo%d'), el => el.style.fill='%@'); document.getElementById('tr%d').textContent = '%@';", sblk, sblk, col, sblk,
              strn ? strn : @""];
        [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
    }
}

- (void) hideTrainInfos
{
    NSString *js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('trinfo'), el => el.style.visibility = 'hidden');"];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];
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

// @property (nonatomic) int highlightIna;
- (int) highlightIna
{
    return _highlightIna;
}
- (void) setHighlightIna:(int)hv
{
    if (_highlightIna == hv) return;
    _highlightIna = hv;
   
    int n = topology_num_sblkd();
    for (int i = 0; i<n; i++) {
        // <polyline id="SBLK05" class="track CANTON3" stroke="#000000" stroke-width="5px" fill="none" points="600,320 680,320 720,280 720,120"></polyline>
        lsblk_num_t ls = {i};
        ina_num_t ina = get_lsblk_ina3221(ls); //uint8_t  get_lsblk_ina3221(lsblk_num_t num);
        int h = (ina.v == hv) ? 1 : 0;
        //NSLog(@"lsblk %d ina %d -> %d", i, ina, h);
         // <polyline id="SBLK05" class="track CANTON3" stroke="#000000" stroke-width="5px" fill="none" points="600,320 680,320 720,280 720,120"></polyline>
        NSString *js = [NSString stringWithFormat:@"document.getElementById('SBLK%2.2d').style['stroke-width'] = '%s';", i, h ? "10px" : "5px"];
        [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
            if (err) {
                NSLog(@"js error : %@\n", err);
            }
        }];
    }
}

- (int) highlightCanton
{
    return _highlightCanton;
}
- (void) setHighlightCanton:(int)c
{
    if (c==_highlightCanton) return;
    
    NSString *js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('track'), el => el.style['stroke-width'] = '5px');"];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];
    js = [NSString stringWithFormat:@"Array.from(document.getElementsByClassName('CANTON%d'), el => el.style['stroke-width'] = '10px');", c];
    [_ctoWebView evaluateJavaScript:js completionHandler:^(id v, NSError *err) {
        if (err) {
            NSLog(@"js error : %@\n", err);
        }
    }];
    _highlightCanton = c;
}

@end
