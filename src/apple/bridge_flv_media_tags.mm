#import "../../private/apple/KBFLVMediaTags.h"
#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/flv/flv_video_mediatag.h"
#include "../../private/flv/flv_audio_mediatag.h"

@interface KBFLVMediaTag()

- (void) fromNative:(const RtmpKit::FLVMediaTag &) tag;

@end

@implementation KBFLVMediaTag

- (void) fromNative:(const RtmpKit::FLVMediaTag &) tag
{
	self.streamId = tag.streamID();
	self.timestamp =  tag.timestamp();
}

@end

@implementation KBFLVAMFMediaTag

- (instancetype) initWithNative:(const RtmpKit::FLVMediaAMFTag &) tag
{
	self = [super init];
	
	if (self)
	{
		[self fromNative:tag];
		
		self.duration = tag.duration();
		self.width = tag.width();
		self.height = tag.height();
		self.videoDataRate = tag.videoDataRate();
		self.frameRate = tag.frameRate();
		self.videoCodecId = tag.videoCodecId();
		self.audioDataRate = tag.audioDataRate();
		self.audioSampleRate = tag.audioSampleRate();
		self.stereo = tag.stereo();
		self.audioCodecId = tag.audioCodecId();
		self.encoder = [NSString stringWithUTF8String:tag.encoder().c_str()];
		self.fileSize = tag.fileSize();
	}
	
	return self;
}

@end

@implementation KBFLVAudioMediaTag

- (instancetype) initWithNative:(const RtmpKit::FLVMediaAudioTag &) tag
{
	self = [super init];
	
	if (self)
	{
		[self fromNative:tag];
		
		const auto &payload = tag.payload();
		self.payload = [NSData dataWithBytes:payload.data() length:payload.size()];
	}
	
	return self;
}

@end


@implementation KBFLVVideoMediaTag


- (instancetype) initWithNative:(const RtmpKit::FLVMediaVideoTag &) tag
{
	self = [super init];
	
	if (self)
	{
		[self fromNative:tag];
		
		const auto &payload = tag.payload();
		self.payload = [NSData dataWithBytes:payload.data() length:payload.size()];
	}
	
	return self;
}

@end
