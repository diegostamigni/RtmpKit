#import "../../private/apple/KBAudioRendererSession.h"
#import "../../private/apple/KBAVAudioSessionManager.h"
#import "../../private/apple/KBUtility.h"

#include "../../private/logger.h"
#include "../../private/buffer_queue.h"
#include "../../private/audio_render_item.h"

#import <AVFoundation/AVFoundation.h>

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
#endif

using namespace std::chrono_literals;

template <typename T>
using QueueType = RtmpKit::BufferQueue<T>;

using QueueItem = RtmpKit::AudioRenderItem;

@interface KBAudioRendererSession()

@property (nonatomic, assign) AudioConverterRef inAudioConverter;
@property (nonatomic, assign) std::shared_ptr<QueueType<QueueItem>> queue;
@property (nonatomic, assign) AudioComponentInstance audioUnit;
@property (nonatomic, strong) NSMutableData *tmpData;
@property (nonatomic, strong) dispatch_queue_t audioDecompressorQueue;

@end

#pragma mark - Renderer callback

static AudioStreamPacketDescription packetDescription;

static OSStatus renderCallback(void *inRefCon,
							   AudioUnitRenderActionFlags *ioActionFlags,
							   const AudioTimeStamp *inTimeStamp,
							   UInt32 inBusNumber,
							   UInt32 inNumberFrames,
							   AudioBufferList *ioData)
{
	OSStatus err = noErr;
	auto bridge = (__bridge KBAudioRendererSession *) inRefCon;
	if (!bridge) return kAudioSilenceErr;
	
    const auto renderEmpty = [](AudioBuffer &buffer) -> OSStatus
	{
		memset(buffer.mData, 0, buffer.mDataByteSize);
        return kAudioSilenceErr;
    };
    
	for (decltype(ioData->mNumberBuffers) i = 0; i < ioData->mNumberBuffers; i++)
	{
		auto& buffer = ioData->mBuffers[i];
		
#if defined TARGET_IOS
		if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive)
		{
			err = renderEmpty(buffer);
			continue;
		}
#endif
	
		if (bridge.queue->empty() || !bridge.queue->isReady())
		{
            err = renderEmpty(buffer);
			continue;
		}
		
		auto item = bridge.queue->pop();
		auto data = item.payload();
		const auto count = item.bufferLenght();
		const auto ts = item.timestamp();
        const auto presentationTimestamp = CMTimeMake(ts, 1000);
		
		if (![bridge.delegate canRenderAudioSampleWithPresentationTime: presentationTimestamp])
		{
			renderEmpty(buffer);
			continue;
		}
		
		[bridge.delegate renderingAudioSampleAtPresentationTime:presentationTimestamp];
		const auto size = MIN(buffer.mDataByteSize, count);
		memmove(buffer.mData, data.get(), size);
		buffer.mDataByteSize = static_cast<decltype(buffer.mDataByteSize)>(size);
	}
	
	return err;
}

#pragma mark - Converter callback

static OSStatus converterCallback(AudioConverterRef,
								  UInt32* count,
								  AudioBufferList* io,
								  AudioStreamPacketDescription** pd,
								  void* inRefCon)
{
	auto bridge = (__bridge KBAudioRendererSession *) inRefCon;
	
	*pd = &packetDescription;
    const auto size = static_cast<decltype(packetDescription.mDataByteSize)>(bridge.tmpData.length);
    
    packetDescription.mDataByteSize = size;
	io->mBuffers[0].mDataByteSize = size;
	io->mBuffers[0].mData = [bridge.tmpData mutableBytes];
	
	return noErr;
}

#pragma mark - objective-c class

@implementation KBAudioRendererSession

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

- (instancetype)init
{
	self = [super init];
	
	if (self)
	{
		[self setup];
	}
	
	return self;
}

- (instancetype)initWithRtmpSettings:(KBRtmpSessionSettings *)settings
{
	self = [self init];
	
	if (self)
	{
		self.rtmpSettings = settings;
		[self setup];
	}
	
	return self;
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
	self.queue->setMinimumBufferSize([self minSizeFromSettingsForBufferQueue]);
}

- (void) networkTypeChanged: (KBNetworkType) networkType
{
	if (KBNetworkConfiguration *networkConf = [self.rtmpSettings networkConfigurationWithNetworkType:networkType])
	{
		self.queue->setMinimumBufferSize(std::chrono::milliseconds(static_cast<int>(1000 * networkConf.bufferSize)));
		self.queue->syncBufferSize();
	
		LOG_DEBUG(boost::format("Current network type has changed: %1%, buffer lenght is: %2%ms")
			  % (RtmpKit::NetworkType) networkType
			  % self.queue->currentBufferSize().count());
	}
}

- (void) setup
{
	memset(&packetDescription, 0, sizeof(packetDescription));
	
	const auto hpAttr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, -1);
	self.audioDecompressorQueue = dispatch_queue_create("AudioDecompressionQueue", hpAttr);

	self.queue = std::make_shared<QueueType<QueueItem>>([self minSizeFromSettingsForBufferQueue]);
}

- (std::chrono::milliseconds) minSizeFromSettingsForBufferQueue
{
	auto minBufSize = std::chrono::milliseconds(500);
	
	if (KBNetworkConfiguration *networkConf = [self.rtmpSettings highestNetworkConfiguration])
	{
		minBufSize = std::chrono::milliseconds(static_cast<int>(networkConf.bufferSize * 1000));
	}
	
	return minBufSize;
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
	
	AudioComponentDescription defaultOutputDescription;
	defaultOutputDescription.componentType = kAudioUnitType_Output;
	defaultOutputDescription.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
	defaultOutputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
	defaultOutputDescription.componentFlags = 0;
	defaultOutputDescription.componentFlagsMask = 0;
	
	AudioComponent defaultOutput = AudioComponentFindNext(NULL, &defaultOutputDescription);
	NSAssert(defaultOutput, @"Can't find default output.");
	
	status = AudioComponentInstanceNew(defaultOutput, &_audioUnit);
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioComponentInstanceNew failed with error: %1%") % status);
	}
	
	status = AudioUnitSetProperty(_audioUnit,
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Output,
								  kOutputBus,
								  &flag,
								  sizeof(flag));
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioUnitSetProperty for EnableIO failed with error: %1%") % status);
	}
	
	AudioStreamBasicDescription audioFormat = [self inputFormatDescription];
	status = AudioUnitSetProperty(_audioUnit,
								  kAudioUnitProperty_StreamFormat,
								  kAudioUnitScope_Input,
								  kOutputBus,
								  &audioFormat,
								  sizeof(audioFormat));
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioUnitSetProperty for StreamFormat failed with error: %1%") % status);
	}
	
	AURenderCallbackStruct callbackStruct = { renderCallback, (__bridge void *) self };
	status = AudioUnitSetProperty(_audioUnit,
								  kAudioUnitProperty_SetRenderCallback,
								  kAudioUnitScope_Global,
								  kOutputBus,
								  &callbackStruct,
								  sizeof(callbackStruct));
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioUnitSetProperty for CallbackStruct failed with error: %1%") % status);
	}
	
	status = AudioUnitInitialize(_audioUnit);
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioUnitInitialize failed with error: %1%") % status);
	}
}

- (void) start
{
	if (self.running) return;
	if (_audioUnit) return;
	
	[self initializeAudioIfNeeded];
	OSStatus status = AudioOutputUnitStart(_audioUnit);
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioOutputUnitStart failed with error: %1%") % status);
		return;
	}
	
	self.running = true;
}

- (void) startIfNeeded
{
	[self start];
}

- (void) reset
{
    self.queue->clear();
}

- (void) end
{
	if (!self.running) return;
	
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
	
	self.queue->clear();
	self.running = false;
	self.audioUnit = nil;
}

- (OSStatus) setupConverterIfNeeded
{
	if (_inAudioConverter) return noErr;
	AudioStreamBasicDescription inputFormat = _audioFormat;
	AudioStreamBasicDescription destinationFormat = [self inputFormatDescription];
	
	AudioClassDescription desc = {
		kAudioDecoderComponentType,
		destinationFormat.mFormatID,
		kAudioUnitManufacturer_Apple
	};
	
	OSStatus status = AudioConverterNewSpecific(&inputFormat, &destinationFormat, 1, &desc, &_inAudioConverter);
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("AudioConverterNewSpecific failed with error: %1%") % status);
		return status;
	}
	
	return status;
}

- (void)setAudioFormat:(AudioStreamBasicDescription)audioFormat
{
	_audioFormat = audioFormat;
	
	if (!_inAudioConverter)
	{
		[self setupConverterIfNeeded];
	}
}

- (void) appendAudioData: (std::vector<uint8_t>) audioData
	withPresentationTime: (CMTime) presentationTime
{
	if (audioData.empty() || !CMTIME_IS_VALID(presentationTime)) return;
	
#if defined TARGET_IOS
	if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive)
	{
		return;
	}
#endif
	
	dispatch_async(self.audioDecompressorQueue,
	^{
		[self setupConverterIfNeeded];
		AudioStreamBasicDescription audioFormat = [self audioFormat];
					   
		auto rawData = decltype(audioData){};
		std::move(audioData.begin(), audioData.end(), std::back_inserter(rawData));
		
		auto *data = rawData.data();
		const auto size = rawData.size();
		
		UInt32 packetsPerBuffer = 0;
		decltype(packetsPerBuffer) outputBufferSize = audioFormat.mFramesPerPacket * 4;
		decltype(packetsPerBuffer) sizePerPacket = audioFormat.mBytesPerPacket;
		
		if (sizePerPacket == 0)
		{
			decltype(packetsPerBuffer) size = sizeof(sizePerPacket);
			AudioConverterGetProperty(_inAudioConverter,
									  kAudioConverterPropertyMaximumOutputPacketSize,
									  &size,
									  &sizePerPacket);
			
			if (sizePerPacket > outputBufferSize)
			{
				outputBufferSize = sizePerPacket;
			}
			
			packetsPerBuffer = outputBufferSize / sizePerPacket;
		}
		else
		{
			packetsPerBuffer = outputBufferSize / sizePerPacket;
		}
		
		if (self.tmpData) self.tmpData = nil;
		self.tmpData = [[NSMutableData alloc] initWithBytesNoCopy:data length:size freeWhenDone:false];
		std::shared_ptr<RtmpKit::u8> buffer(new RtmpKit::u8[outputBufferSize]);
		
		AudioBufferList io;
		io.mNumberBuffers = 1;
		io.mBuffers[0].mNumberChannels = 1;
		io.mBuffers[0].mDataByteSize = outputBufferSize;
		io.mBuffers[0].mData = buffer.get();
		
		const auto err = AudioConverterFillComplexBuffer(_inAudioConverter,
														 converterCallback,
														 (__bridge void*) self,
														 &packetsPerBuffer,
														 &io,
														 nullptr);
		
		if (err != noErr)
		{
			LOG_DEBUG(boost::format("Error while converting the audio input: %1%") % err);
			return;
		}
		
		RtmpKit::AudioRenderItem item{
			static_cast<RtmpKit::u24>(CMTimeGetSeconds(presentationTime) * 1000),
			static_cast<RtmpKit::u24>(std::ceil((audioFormat.mFramesPerPacket / audioFormat.mSampleRate) * 1000)),
			packetsPerBuffer,
			buffer
		};
		
		self.queue->push(std::move(item));
	});
}

@end
