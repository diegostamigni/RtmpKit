#import "../../private/apple/KBAVAudioSessionManager.h"
#import "../../private/apple/KBUtility.h"

#include "../../private/logger.h"

@interface KBAVAudioSessionManager()
{
}
@property (nonatomic, assign) BOOL active;
@property (nonatomic, assign) BOOL recording;
@property (nonatomic, assign) NSInteger activeCount;
@end

@implementation KBAVAudioSessionManager
@synthesize recording = _recording;
@synthesize active = _active;

+ (instancetype) sharedInstance
{
	static dispatch_once_t p = 0;
	__strong static id _sharedObject = nil;
	dispatch_once(&p, ^{ _sharedObject = [[self alloc] init]; });
	return _sharedObject;
}

- (instancetype) init
{
	self = [super init];
	
	if (self)
	{
		[self attachObserver];
		
		[[self audioSession] setPreferredInputNumberOfChannels:kChannelCount error:nil];
		[[self audioSession] setPreferredOutputNumberOfChannels:kChannelCount error:nil];

		[self setPreferredSampleRate:kSampleRate];
		[self setPreferredIOBufferLength:kFrameLenght];
	}
	
	return self;
}

- (AVAudioSession *) audioSession
{
	return [AVAudioSession sharedInstance];
}

- (void) detachObserver
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) attachObserver
{
#if defined TARGET_IOS
	[self detachObserver];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(handleInterruptionState:)
												 name:AVAudioSessionInterruptionNotification
											   object:nil];
#endif
}

- (void)setRecording:(BOOL)recording
{
	_recording = recording;
}

# pragma mark - Utilities

- (void) prepareForRecordingIfNeeded
{
	if (![self isRecording])
	{
		[self switchToCategoryPlayAndRecord];
	}
}

- (void) prepareForPlaybackIfNeeded
{
	if (![self isRecording])
	{
		[self switchToCategoryPlayback];
	}
}

- (void) increaseActiveCounter
{
	if (_activeCount == 2) return;
	_activeCount += 1;
}

- (void) decreaseActiveCounter
{
	if (_activeCount == 0) return;
	_activeCount -= 1;
}

# pragma mark - Configuration

- (void) setPreferredSampleRate: (double) rate
{
	NSError *err;
	[[self audioSession] setPreferredSampleRate:rate error:&err];
	
	if (err != nil)
	{
		LOG_DEBUG(boost::format("setPreferredSampleRate failed with error: %1%") % [err debugDescription]);
	}
}

- (void) setPreferredIOBufferDuration: (double) duration
{
	NSError *err;
	[[self audioSession] setPreferredIOBufferDuration:duration error:&err];
	
	if (err != nil)
	{
		LOG_DEBUG(boost::format("setPreferredIOBufferDuration failed with error: %1%") % [err debugDescription]);
	}
}

- (void) setPreferredIOBufferLength: (int) count
{
	[self setPreferredIOBufferDuration:[self durationForFrameLength:count withSampleRate: [self preferredSampleRate]]];
}

- (double) preferredSampleRate
{
	return [[self audioSession] preferredSampleRate];
}

- (double) durationForFrameLength: (int) n withSampleRate: (double) rate
{
	return (double(n) + 0.5) / rate;
}

# pragma mark - Switch Categories

- (void) switchToCategoryPlayAndRecord
{
	if ([[self audioSession] category] != AVAudioSessionCategoryPlayAndRecord)
	{
		[self switchToCategory:AVAudioSessionCategoryPlayAndRecord];
		[self switchToMode:AVAudioSessionModeVideoChat];
		[self setRecording:true];
	}
}

- (void) switchToCategoryPlayback
{
	if ([[self audioSession] category] != AVAudioSessionCategoryPlayback)
	{
		[self switchToCategory:AVAudioSessionCategoryPlayback];
		[self switchToMode:AVAudioSessionModeDefault];
		[self setRecording:false];
	}
}

- (void) switchToCategoryRecord
{
	if ([[self audioSession] category] != AVAudioSessionCategoryRecord)
	{
		[self switchToCategory:AVAudioSessionCategoryRecord];
		[self switchToMode:AVAudioSessionModeVideoRecording];
		[self setRecording:true];
	}
}

- (void) switchToCategory: (NSString *) category
{
	AVAudioSessionCategoryOptions options = AVAudioSessionCategoryOptionMixWithOthers;
	
	if (category == AVAudioSessionCategoryPlayAndRecord)
	{
		options |= AVAudioSessionCategoryOptionAllowBluetooth;
		options |= AVAudioSessionCategoryOptionDefaultToSpeaker;
	}
	
	NSError *err;
	[[self audioSession] setCategory:category
						 withOptions:options
							   error:&err];
	
	if (err != nil)
	{
		LOG_DEBUG(boost::format("setCategory failed with error: %1%") % [err debugDescription]);
	}
}

# pragma mark - Switch Mode

- (void) switchToMode:(NSString *) mode
{
	NSError *err;
	[[self audioSession] setMode:mode error:&err];
	
	if (err != nil)
	{
		LOG_DEBUG(boost::format("setMode failed with error: %1%") % [err debugDescription]);
	}
}

# pragma mark - Others

- (BOOL) canDeactivate
{
	return _activeCount < 1;
}

- (BOOL) isActive
{
	return _activeCount > 0;
}

- (BOOL) isRecording
{
	return [self isActive] && _recording;
}

- (void)setActive:(BOOL)active
{
	if (!active)
	{
		[self decreaseActiveCounter];
	}
	else
	{
		[self increaseActiveCounter];
	}
	
	if (active != _active)
	{
		if (!active && ![self canDeactivate])
		{
			return;
		}
		
		NSError *err;
		[[self audioSession] setActive:active
						   withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation
								 error:&err];

		if (active)
		{
			[self attachObserver];
		}
		else
		{
			[self detachObserver];
		}
		
		if (err != nil)
		{
			LOG_DEBUG(boost::format("setActive failed with error: %1%") % [err debugDescription]);
			return;
		}
		
		_active = active;
	}
}

- (BOOL)isOtherAudioPlaying
{
	return [[self audioSession] isOtherAudioPlaying];
}

#pragma mark - AVAudioSessionDelegate

- (void) handleInterruptionState: (NSNotification *) notification
{
	NSNumber *interruptionType = [[notification userInfo] objectForKey:AVAudioSessionInterruptionTypeKey];
	NSNumber *interruptionOption = [[notification userInfo] objectForKey:AVAudioSessionInterruptionOptionKey];
	
	switch (interruptionType.unsignedIntegerValue)
	{
		case AVAudioSessionInterruptionTypeBegan:
		{
			LOG_DEBUG("AVAudioSessionInterruptionTypeBegan");
			
			if (self.delegate)
			{
				[self.delegate audioSessionManager:self audioInterruptionBegan:AVAudioSessionInterruptionTypeBegan];
			}
		} break;
			
		case AVAudioSessionInterruptionTypeEnded:
		{
			LOG_DEBUG("AVAudioSessionInterruptionTypeEnded");
			
			if (self.delegate)
			{
				[self.delegate audioSessionManager:self audioInterruptionEnded:AVAudioSessionInterruptionTypeEnded];
			}
			
			if (interruptionOption.unsignedIntegerValue == AVAudioSessionInterruptionOptionShouldResume)
			{
				LOG_DEBUG("AVAudioSessionInterruptionOptionShouldResume");
				
				if (self.delegate)
				{
					[self.delegate audioSessionManager:self
						 audioInterruptionShouldResume:AVAudioSessionInterruptionOptionShouldResume];
				}
			}
		} break;
			
		default:
			break;
	}
}

@end
