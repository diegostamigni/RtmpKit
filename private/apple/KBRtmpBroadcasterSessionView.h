//
//  KBRtmpBroadcasterSessionView.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 14/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "KBRtmpSessionSettings.h"
#import "KBBroadcasterSessionDelegate.h"

#if defined TARGET_IOS
#import <UIKit/UIKit.h>
@interface KBRtmpBroadcasterSessionView : UIView
#else
#import <AppKit/AppKit.h>
@interface KBRtmpBroadcasterSessionView : NSView
#endif

@property (atomic, assign, getter=isRunning, readonly) BOOL running;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;
@property (nonatomic, weak, nullable) id<KBBroadcasterSessionDelegate> delegate;

- (void)startWithHost: (NSString *_Nonnull) host
			  andPort: (NSString *_Nonnull) port
		   andAppName: (NSString *_Nonnull) appName
		 andStreamURL: (NSString *_Nonnull) streamURL;

- (void) end;
- (void) switchCamera;

@end
