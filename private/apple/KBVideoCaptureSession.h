//
//  KBVideoCaptureSession.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBRtmpSessionSettings.h"

#import "../../private/apple/KBVideoCompressorSessionDelegate.h"
#import "../../private/apple/KBVideoCompressorSession.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <QuartzCore/QuartzCore.h>

@interface KBVideoCaptureSession : NSObject
@property (nonatomic, weak, nullable) id<KBVideoCompressorSessionDelegate> delegate;
@property (nonatomic, strong, nonnull) AVCaptureVideoPreviewLayer *layerOutput;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;

- (instancetype _Nonnull) initUsingFrontCamera:(BOOL) useFrontCamera;

- (void) switchCamera;
- (void) startIfNeeded;
- (void) start;
- (void) end;
- (void) reset;
- (void) fixCameraOrientation;

- (KBVideoCompressorSession *_Nullable)videoCompressorSession;

@end
