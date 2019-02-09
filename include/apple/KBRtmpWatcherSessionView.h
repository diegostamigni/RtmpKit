//
//  KBRtmpWatcherSessionView.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 25/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "KBRtmpSessionSettings.h"
#import "KBWatcherSessionDelegate.h"
#import "KBSession.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
	@interface KBRtmpWatcherSessionView : UIView <KBRtmpWatcherSession>
#else
	#import <AppKit/AppKit.h>
	@interface KBRtmpWatcherSessionView : NSView <KBRtmpWatcherSession>
#endif

@property (atomic, assign, getter=isRunning, readonly) BOOL running;
@property (nonatomic, assign, getter=isMirroredHorizontally) BOOL mirrorHorizontally;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;
@property (nonatomic, weak, nullable) id<KBWatcherSessionDelegate> delegate;
@property (nonatomic, getter=isLayoutForOnStage) BOOL layoutForOnStage;

- (void)startWithHost: (NSString *_Nonnull) host
			  andPort: (NSString *_Nonnull) port
		   andAppName: (NSString *_Nonnull) appName
		 andStreamURL: (NSString *_Nonnull) streamURL
	 layoutForOnStage: (BOOL) onStage;

- (void) end;
- (void) realign;

@end
