//
//  KBVideoDecompressorSession.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBRtmpSessionSettings.h"

#import "../../private/apple/KBVideoDecompressorSessionDelegate.h"

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

@interface KBVideoDecompressorSession : NSObject
@property (nonatomic, weak, nullable) id<KBVideoDecompressorSessionDelegate> delegate;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;

- (void) end;

- (instancetype _Nonnull) initWithRtmpSettings: (KBRtmpSessionSettings *_Nullable) settings;

- (void) decodeFrameWithSampleBuffer: (CMSampleBufferRef _Nonnull) sampleBuffer;

- (void) decodeFrameWithSampleBuffer: (CMSampleBufferRef _Nonnull) sampleBuffer
				andFormatDescription: (CMFormatDescriptionRef _Nullable) formatDescription;

@end
