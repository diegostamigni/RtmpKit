#import "../../private/apple/KBAudioCaptureSession.h"
#import "../../private/apple/KBAVAudioSessionManager.h"
#import "../../private/apple/KBUtility.h"
#include "../../private/logger.h"

#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface KBAudioCaptureSession()
{
}

@property (nonatomic, assign) AudioConverterRef inAudioConverter;
@property (nonatomic, assign) AudioComponentInstance audioUnit;
@property (nonatomic, assign) double audioSampleCount;
@property (nonatomic, assign) BOOL running;
@property (nonatomic, assign) AudioBuffer converterCurrentBuffer;

- (void) processBuffer: (AudioBufferList*) audioBufferList
         withTimestamp: (const AudioTimeStamp *) timestamp;

@end

#pragma mark Recording callback

static OSStatus recordingCallback(void *inRefCon,
								  AudioUnitRenderActionFlags *ioActionFlags,
								  const AudioTimeStamp *inTimeStamp,
								  UInt32 inBusNumber,
								  UInt32 inNumberFrames,
								  AudioBufferList *ioData)
{
	auto audioProcessor = (__bridge KBAudioCaptureSession *) inRefCon;
	if (!audioProcessor) return -1;
	
	AudioBufferList bufferList;
	bufferList.mNumberBuffers = 1;
	bufferList.mBuffers[0].mNumberChannels = kChannelCount;
	bufferList.mBuffers[0].mDataByteSize = inNumberFrames * sizeof(SInt16) * 2;
	bufferList.mBuffers[0].mData = malloc(bufferList.mBuffers[0].mDataByteSize);
	
	OSStatus err = AudioUnitRender([audioProcessor audioUnit],
								   ioActionFlags,
								   inTimeStamp,
								   kInputBus,
								   inNumberFrames,
								   &bufferList);
	
	
	[audioProcessor processBuffer:&bufferList withTimestamp: inTimeStamp];
	free(bufferList.mBuffers[0].mData);
	
	return err;
}

static OSStatus converterCallback(AudioConverterRef,
								  UInt32* count,
								  AudioBufferList* io,
								  AudioStreamPacketDescription** pd,
								  void* inRefCon)
{
	auto audioProcessor = (__bridge KBAudioCaptureSession *) inRefCon;
	
	io->mBuffers[0].mNumberChannels = 1;
	io->mBuffers[0].mDataByteSize = audioProcessor.converterCurrentBuffer.mDataByteSize;
	io->mBuffers[0].mData = static_cast<uint8_t*>(audioProcessor.converterCurrentBuffer.mData);
	
	return noErr;
}

#pragma mark objective-c class

@implementation KBAudioCaptureSession

- (instancetype) init
{
	self = [super init];
	
	if (self)
	{
        [self setup];
	}
	
	return self;
}

- (instancetype) initWithRtmpSettings:(KBRtmpSessionSettings *) settings
{
    self = [super init];
    
    if (self)
    {
        self.rtmpSettings = settings;
        [self setup];
    }
    
    return self;
}

- (AudioStreamBasicDescription) inputFormatDescription
{
	AudioStreamBasicDescription audioFormat;
	memset(&audioFormat, 0, sizeof(audioFormat));
	
	audioFormat.mSampleRate = kSampleRate;
	audioFormat.mFormatID = kAudioFormatLinearPCM;
	audioFormat.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
	audioFormat.mChannelsPerFrame = kChannelCount;
	audioFormat.mBitsPerChannel = kSampleSize;
	audioFormat.mFramesPerPacket = 1;
	audioFormat.mBytesPerPacket = 2;
	audioFormat.mBytesPerFrame = 2;
	audioFormat.mReserved = 0;
	
	return audioFormat;
}

- (AudioStreamBasicDescription) outputFormatDescription
{
	AudioStreamBasicDescription audioFormat;
	memset(&audioFormat, 0, sizeof(audioFormat));
	
	audioFormat.mChannelsPerFrame = kChannelCount;
	audioFormat.mFormatID = kAudioFormatMPEG4AAC;
	audioFormat.mSampleRate = kSampleRate;
	audioFormat.mFormatFlags = kMPEG4Object_AAC_LC;
	
	return audioFormat;
}

- (void) setup
{
	if([[AVAudioSession sharedInstance] respondsToSelector:@selector(requestRecordPermission:)])
	{
		[[AVAudioSession sharedInstance] requestRecordPermission:^(BOOL granted)
		{
			if (granted)
			{
				[self start];
			}
		}];
	}
}

- (KBAVAudioSessionManager *) audioSession
{
	return [KBAVAudioSessionManager sharedInstance];
}

- (void) initializeAudioIfNeeded
{
	if (_audioUnit) return;
	
	UInt32 flag = 1; // { 1: ON, 0: OFF }
	OSStatus status = noErr;
	
	[[self audioSession] prepareForRecordingIfNeeded];
	[[self audioSession] setActive:true];
	
	AudioComponentDescription desc;
	memset(&desc, 0, sizeof(desc));
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
 
	AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
 
	status = AudioComponentInstanceNew(inputComponent, &_audioUnit);
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioComponentInstanceNew failed with error: %1%") % status);
		return;
	}
 
	status = AudioUnitSetProperty(_audioUnit,
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Input,
								  kInputBus,
								  &flag,
								  sizeof(flag));
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioUnitSetProperty failed with error: %1%") % status);
	}
 
	AudioStreamBasicDescription audioFormat = [self inputFormatDescription];
	
	status = AudioUnitSetProperty(_audioUnit,
								  kAudioUnitProperty_StreamFormat,
								  kAudioUnitScope_Output,
								  kInputBus,
								  &audioFormat,
								  sizeof(audioFormat));
	
	if (status != noErr)
    {
        LOG_DEBUG(boost::format("AudioUnitSetProperty failed with error: %1%") % status);
	}
 
	AURenderCallbackStruct callbackStruct = { recordingCallback, (__bridge void*) self };
 
	status = AudioUnitSetProperty(_audioUnit,
								  kAudioOutputUnitProperty_SetInputCallback,
								  kAudioUnitScope_Global,
								  kInputBus,
								  &callbackStruct,
								  sizeof(callbackStruct));
	
	if (status != noErr)
    {
        LOG_DEBUG(boost::format("AudioUnitSetProperty failed with error: %1%") % status);
	}
	
	status = AudioUnitInitialize(_audioUnit);
	
	if (status != noErr)
    {
        LOG_DEBUG(boost::format("AudioUnitSetProperty failed with error: %1%") % status);
		self.running = false;
		return;
	}
}

- (OSStatus) setupConverterIfNeeded
{
	if (_inAudioConverter) return noErr;
	AudioStreamBasicDescription inputFormat = [self inputFormatDescription];
	AudioStreamBasicDescription destinationFormat = [self outputFormatDescription];
	AudioClassDescription desc = { kAudioEncoderComponentType, destinationFormat.mFormatID, kAudioUnitManufacturer_Apple };
	
	OSStatus status = AudioConverterNewSpecific(&inputFormat, &destinationFormat, 1, &desc, &_inAudioConverter);
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioConverterNewSpecific failed with error: %1%") % status);
		return status;
	}
	
	UInt32 bitRate = kBitRate;
	status = AudioConverterSetProperty(_inAudioConverter, kAudioConverterEncodeBitRate, sizeof(bitRate), &bitRate);
	
	if (status != noErr)
    {
        LOG_DEBUG(boost::format("AudioConverterSetProperty failed with error: %1%") % status);
		return status;
	}
	
	return status;
}

#pragma mark controll stream

- (void)startIfNeeded
{
	if (!self.running)
	{
		[self start];
	}
}

- (void) start
{
	[self initializeAudioIfNeeded];
	
	if (!_audioUnit) return;
	OSStatus status = AudioOutputUnitStart(_audioUnit);
	
	if (status != noErr)
    {
        LOG_DEBUG(boost::format("AudioOutputUnitStart failed with error: %1%") % status);
		self.running = false;
		return;
	}
	
	self.running = true;
}

- (void) end
{
	if (!_audioUnit) return;
	if (self.running == false) return;
	
	OSStatus status = AudioOutputUnitStop(_audioUnit);
	if (status != noErr)
    {
        LOG_DEBUG(boost::format("AudioOutputUnitStop failed with error: %1%") % status);
		return;
	}
	
	[self.audioSession setActive:false];
	
	if (_inAudioConverter)
	{
		AudioConverterReset(_inAudioConverter);
		_inAudioConverter = nil;
	}
	
	self.running = false;
	self.audioUnit = nil;
}

- (void) reset
{
	self.audioSampleCount = 0;
}

#pragma mark processing

- (void) processBuffer: (AudioBufferList*) audioBufferList withTimestamp:(const AudioTimeStamp *)timestamp
{
	if (!audioBufferList || !timestamp) return;
	const auto sourceBuffer = audioBufferList->mBuffers[0];
	
	if (!sourceBuffer.mData) return;
	UInt32 count = 1;
	
	self.converterCurrentBuffer = AudioBuffer { kChannelCount, sourceBuffer.mDataByteSize, sourceBuffer.mData };
	
	AudioBufferList io;
	io.mNumberBuffers = kChannelCount;
	io.mBuffers[0] = sourceBuffer;
	
	[self setupConverterIfNeeded];
	const auto err = AudioConverterFillComplexBuffer(_inAudioConverter,
													 converterCallback,
													 (__bridge void*) self,
													 &count,
													 &io,
													 nullptr);
	
	if (err != noErr)
    {
        LOG_DEBUG(boost::format("Error while converting the audio input: %1%") % err);
		return;
	}
	
	const auto data = static_cast<uint8_t*>(io.mBuffers[0].mData);
	const auto lenght = io.mBuffers[0].mDataByteSize;
	const auto ts = static_cast<uint32_t>((self.audioSampleCount / kSampleRate) * 1000);
	
	if (self.delegate &&
		[self.delegate respondsToSelector:@selector(audioSampleReceived:
													withLenght:
													withPresentationTime:
													andDuration:)])
	{
		[self.delegate audioSampleReceived:data
								withLenght:lenght
					  withPresentationTime:ts
							   andDuration:kCMTimeInvalid];
	}
	
	self.audioSampleCount += kFrameLenght;
}

@end
