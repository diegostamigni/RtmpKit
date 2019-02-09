//
//  KBRtmpWatcherSessionDelegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 25/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol KBWatcherSessionDelegate <NSObject>

- (void) sessionStarted;
- (void) sessionEnded;
- (void) sessionIsReconnecting;

@end
