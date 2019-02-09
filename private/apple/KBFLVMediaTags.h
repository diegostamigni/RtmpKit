//
//  KBFLVTags.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 19/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>

#if defined __cplusplus
namespace RtmpKit
{
	class FLVMediaAMFTag;
	class FLVMediaAudioTag;
	class FLVMediaVideoTag;
} // end RtmpKit
#endif

@interface KBFLVMediaTag : NSObject
@property (nonatomic, assign) UInt32 timestamp;
@property (nonatomic, assign) UInt32 streamId;
@property (nonatomic, strong, nullable) NSData *payload;
@end

@interface KBFLVAMFMediaTag : KBFLVMediaTag
@property (nonatomic, assign) NSTimeInterval duration;
@property (nonatomic, assign) double width;
@property (nonatomic, assign) double height;
@property (nonatomic, assign) double videoDataRate;
@property (nonatomic, assign) double frameRate;
@property (nonatomic, assign) double videoCodecId;
@property (nonatomic, assign) double audioDataRate;
@property (nonatomic, assign) double audioSampleRate;
@property (nonatomic, assign) double audioSampleSize;
@property (nonatomic, assign, getter=isStereo) BOOL stereo;
@property (nonatomic, assign) double audioCodecId;
@property (nonatomic, strong, nullable) NSString *encoder;
@property (nonatomic, assign) double fileSize;

#if defined __cplusplus
- (_Nonnull instancetype) initWithNative:(const RtmpKit::FLVMediaAMFTag &) tag;
#endif
@end

@interface KBFLVAudioMediaTag : KBFLVMediaTag
#if defined __cplusplus
- (_Nonnull instancetype) initWithNative:(const RtmpKit::FLVMediaAudioTag &) tag;
#endif
@end


@interface KBFLVVideoMediaTag : KBFLVMediaTag
#if defined __cplusplus
- (_Nonnull instancetype) initWithNative:(const RtmpKit::FLVMediaVideoTag &) tag;
#endif
@end
