//
//  KBOpenGLBasedPreviewView.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBRtmpSessionSettings.h"

#import "../../private/apple/KBAudioRendererSessionDelegate.h"

#import <CoreMedia/CoreMedia.h>

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
	#import <GLKit/GLKit.h>
	@interface KBOpenGLBasedPreviewView : GLKView <GLKViewDelegate, KBAudioRendererSessionDelegate>
#else
	#import <AppKit/AppKit.h>
	@interface KBOpenGLBasedPreviewView : NSView
#endif

@property (nonatomic, assign, getter=isRenderingImages, readonly) BOOL renderingImages;
@property (nonatomic, getter=isMirroredHorizontally) BOOL mirrorHorizontally;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;
@property (nonatomic, getter=isLayoutForOnStage) BOOL layoutForOnStage;

- (void) appendImage:(CIImage *_Nullable) image
withPresentationTime: (CMTime) presentationTime
		 andTimebase: (CMTimebaseRef _Nonnull) timeBase;

- (void) start;
- (void) end;
- (void) reset;

@end
