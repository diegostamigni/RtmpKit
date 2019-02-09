//
//  KBVideoCompressorSession.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBRtmpSessionSettings.h"

#import "../../private/apple/KBVideoCompressorSessionDelegate.h"

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

@interface KBVideoCompressorSession : NSObject
@property (nonatomic, weak, nullable) id<KBVideoCompressorSessionDelegate> delegate;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;

- (instancetype _Nonnull) initWithCodecType:(CMVideoCodecType) type
							andRtmpSettings: (KBRtmpSessionSettings *_Nonnull) settings;

- (void) processSampleBuffer:(CMSampleBufferRef _Nullable) sampleBuffer;
- (void) reset;
- (void) end;
- (void) updateCompressorBitRate:(double) bitrate;
- (double) resetCompressorBitRate;

@end
