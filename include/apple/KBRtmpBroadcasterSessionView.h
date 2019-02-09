//
//  KBRtmpBroadcasterSessionView.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 14/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "KBLibConfig.h"
#import "KBRtmpSessionSettings.h"
#import "KBBroadcasterSessionDelegate.h"
#import "KBSession.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
	@interface KBRtmpBroadcasterSessionView : UIView <KBRtmpBroadcastSession>
#else
	#import <AppKit/AppKit.h>
	@interface KBRtmpBroadcasterSessionView : NSView <KBRtmpBroadcastSession>
#endif

@property (atomic, assign, getter=isRunning, readonly) BOOL running;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;
@property (nonatomic, weak, nullable) id<KBBroadcasterSessionDelegate> delegate;
@property (nonatomic, getter=isMirroredHorizontally) BOOL mirrorHorizontally;
@property (nonatomic, getter=isLayoutForOnStage) BOOL layoutForOnStage;
@property (nonatomic, getter=isAudioOnly) BOOL audioOnly;
@property (nonatomic) KBVideoCaptureSessionViewOnStageSide onStageSide;

- (void)startWithHost: (NSString *_Nonnull) host
			  andPort: (NSString *_Nonnull) port
		   andAppName: (NSString *_Nonnull) appName
		 andStreamURL: (NSString *_Nonnull) streamURL
	 layoutForOnStage: (BOOL) onStage;

- (void) end;
- (void) switchCamera;
- (void) realign;

@end
