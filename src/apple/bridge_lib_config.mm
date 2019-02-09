#import "../../include/apple/KBLibConfig.h"
#include "../../include/version.h"

@implementation KBLibConfig

+ (NSString *) version
{
    return [NSString stringWithUTF8String: RTMPKIT_VERSION];
}

@end
