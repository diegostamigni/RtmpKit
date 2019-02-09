#import "../../include/apple/KBRtmpSessionSettings.h"

#include "../../private/rtmp/session/rtmp_session_settings.h"

@interface KBRtmpSessionSettings()
{
	RtmpKit::RtmpSessionSettings rtmpSettings;
}
@end

@implementation KBRtmpSessionSettings

- (instancetype)init
{
	if (self = [super init])
	{
		self.networkProperties = [NSSet setWithObjects:
			[[KBNetworkConfiguration alloc] initWithStatus:KBNetworkTypeVeryLow andBufferSize:2],
			[[KBNetworkConfiguration alloc] initWithStatus:KBNetworkTypeLow andBufferSize:1.5],
			[[KBNetworkConfiguration alloc] initWithStatus:KBNetworkTypeMedium andBufferSize:1],
			[[KBNetworkConfiguration alloc] initWithStatus:KBNetworkTypeHigh andBufferSize:0.5],
			[[KBNetworkConfiguration alloc] initWithStatus:KBNetworkTypeHighest andBufferSize:0.2],
		nil];
	}
	
	return self;
}

- (void)setFrameRate:(double)frameRate
{
	rtmpSettings.setFrameRate(frameRate);
}

- (double)frameRate
{
	return rtmpSettings.frameRate();
}

- (void)setAudioChannels:(double)audioChannels
{
	rtmpSettings.setAudioChannels(audioChannels);
}

- (double)audioChannels
{
	return rtmpSettings.audioChannels();
}

- (void)setVideoDataRate:(double)videoDataRate
{
	if (videoDataRate == 0) return;
	rtmpSettings.setVideoDataRate(videoDataRate / 1024.);
}

- (double)videoDataRate
{
	return rtmpSettings.videoDataRate();
}

- (void)setKeyFrameInterval:(double)keyFrameInterval
{
	rtmpSettings.setVideoFrameRate(keyFrameInterval);
}

- (double)keyFrameInterval
{
	return rtmpSettings.videoFrameRate();
}

- (void)setWidth:(double)width
{
	rtmpSettings.setWidth(width);
}

- (double)width
{
	return rtmpSettings.width();
}

- (void)setHeight:(double)height
{
	rtmpSettings.setHeight(height);
}

- (double)height
{
	return rtmpSettings.height();
}

- (KBNetworkConfiguration *_Nullable) lowestNetworkConfiguration
{
	return [self networkConfigurationWithNetworkType: KBNetworkTypeVeryLow];
}

- (KBNetworkConfiguration *_Nullable) highestNetworkConfiguration
{
	return [self networkConfigurationWithNetworkType: KBNetworkTypeHighest];
}

- (KBNetworkConfiguration *_Nullable) networkConfigurationWithNetworkType: (KBNetworkType) networkType
{
	if (!self.networkProperties)
	{
		return nil;
	}
	
	NSPredicate *predicate = [NSPredicate predicateWithFormat:@"status == %d", networkType];
	NSSet<KBNetworkConfiguration*> *results = [self.networkProperties filteredSetUsingPredicate:predicate];
	
	if (results.count != 1)
	{
		return nil;
	}
	
	return [results anyObject];
}

@end
