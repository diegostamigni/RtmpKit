//
//  KBBroadcasterSessionDelegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 21/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol KBBroadcasterSessionDelegate <NSObject>

- (void) sessionStarted;
- (void) sessionEnded;
- (void) sessionIsReconnecting;
- (void) dynamicBitRateDidChange: (NSUInteger) value;

@end
