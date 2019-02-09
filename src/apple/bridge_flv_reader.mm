#import "../../private/apple/KBFLVMediaTags.h"
#import "../../private/apple/KBFLVReader.h"
#include "../../private/reader.h"
#include "../../private/flv/flv_mediaitem.h"

namespace RtmpKit
{
	class FLVMediaItemDelegateCollector : public FLVMediaItemParserDelegate
	{
	protected:
		void amfPacketReady(FLVMediaAMFTag tag) override
		{
			if (objcDel_ && [objcDel_ respondsToSelector:@selector(AMFPacketReceived:)])
			{
				KBFLVAMFMediaTag *objcTag = [[KBFLVAMFMediaTag alloc] initWithNative: tag];
				[objcDel_ AMFPacketReceived: objcTag];
			}
		}
		
		void audioPacketReady(FLVMediaAudioTag tag) override
		{
			if (objcDel_ && [objcDel_ respondsToSelector:@selector(audioPacketReceived:)])
			{
				KBFLVAudioMediaTag *objcTag = [[KBFLVAudioMediaTag alloc] initWithNative: tag];
				[objcDel_ audioPacketReceived: objcTag];
			}
		}
		
		void videoPacketReady(FLVMediaVideoTag tag) override
		{
			if (objcDel_ && [objcDel_ respondsToSelector:@selector(videoPacketReceived:)])
			{
				KBFLVVideoMediaTag *objcTag = [[KBFLVVideoMediaTag alloc] initWithNative: tag];
				[objcDel_ videoPacketReceived: objcTag];
			}
		}
		
		void parsingCompleted() override
		{
			if (objcDel_ && [objcDel_ respondsToSelector:@selector(parsingCompleted)])
			{
				[objcDel_ parsingCompleted];
			}
		}
		
	public:
		__weak id<KBFLVReaderDelegate> objcDel_;
	};
}

@interface KBFLVReader()
{
	std::shared_ptr<RtmpKit::FLVMediaItemDelegateCollector> collector;
	std::unique_ptr<RtmpKit::FileReader> fileReader;
}
@end

@implementation KBFLVReader

- (instancetype)init
{
	self = [super init];
	
	if (self)
	{
		self->collector = std::make_shared<RtmpKit::FLVMediaItemDelegateCollector>();
	}
	
	return self;
}

- (BOOL)read: (NSString *) videoFile
{
	auto fileManger = [NSFileManager defaultManager];
	
	if ([fileManger fileExistsAtPath:videoFile])
	{
		if (self->fileReader != nullptr)
		{
			self->fileReader->close();
			self->fileReader.release();
		}
		
		self->fileReader = std::make_unique<RtmpKit::FileReader>([videoFile cStringUsingEncoding:NSUTF8StringEncoding]);
		
		auto readResult = self->fileReader->readAll();
		const auto &data = readResult.second;
		
		RtmpKit::FLVMediaItem flvItem;
		self->collector->objcDel_ = [self delegate];
		
		flvItem.addDelegate(self->collector);
		flvItem.deserialize(data.cbegin(), data.cend());
		self->fileReader->close();
		
		return (flvItem.header().signature() == RtmpKit::supportedFileTypes.at(RtmpKit::FileType::FLV));
	}
	
	return false;
}

@end
