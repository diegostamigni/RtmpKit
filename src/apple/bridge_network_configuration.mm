#import "../../include/apple/KBNetworkConfiguration.h"

@implementation KBNetworkConfiguration

- (instancetype) initWithStatus: (KBNetworkType) status andBufferSize: (NSTimeInterval) size
{
	if (self = [super init])
	{
		self.status = status;
		self.bufferSize = size;
	}
	
	return self;
}

- (BOOL)isEqual:(KBNetworkConfiguration *) object
{
	return object && ([object status] == [self status]);
}

@end
