//
//  KBVideoCaptureSessionView.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 13/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBRtmpSessionSettings.h"
#import "../../include/apple/KBLibConfig.h"

#import "../../private/apple/KBVideoCompressorSessionDelegate.h"
#import "../../private/apple/KBVideoCompressorSession.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
	@interface KBVideoCaptureSessionView : UIView
#else
	#import <AppKit/AppKit.h>
	@interface KBVideoCaptureSessionView : NSView
#endif

@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;
@property (nonatomic, weak, nullable) id<KBVideoCompressorSessionDelegate> delegate;
@property (nonatomic, getter=isMirroredHorizontally) BOOL mirrorHorizontally;
@property (nonatomic, getter=isLayoutForOnStage) BOOL layoutForOnStage;
@property (nonatomic) KBVideoCaptureSessionViewOnStageSide onStageSide;

- (void) switchCamera;
- (void) start;
- (void) startIfNeeded;
- (void) end;
- (void) reset;
- (void) fixCameraOrientation;

- (KBVideoCompressorSession *_Nullable) videoCompressorSession;

@end
