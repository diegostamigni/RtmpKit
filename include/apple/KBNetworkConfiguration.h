//
//  KBNetworkConfiguration.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 17/02/2017.
//  Copyright © 2017 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>

#if defined __cplusplus

#include "../network_types.h"
#include <type_traits>

typedef NS_ENUM(NSInteger, KBNetworkType)
{
	KBNetworkTypeVeryLow = std::underlying_type_t<RtmpKit::NetworkType>(RtmpKit::NetworkType::VeryLow),
	KBNetworkTypeLow = std::underlying_type_t<RtmpKit::NetworkType>(RtmpKit::NetworkType::Low),
	KBNetworkTypeMedium = std::underlying_type_t<RtmpKit::NetworkType>(RtmpKit::NetworkType::Medium),
	KBNetworkTypeHigh = std::underlying_type_t<RtmpKit::NetworkType>(RtmpKit::NetworkType::High),
	KBNetworkTypeHighest = std::underlying_type_t<RtmpKit::NetworkType>(RtmpKit::NetworkType::Highest)
};

#else

typedef NS_ENUM(NSInteger, KBNetworkType)
{
	KBNetworkTypeVeryLow = 1,
	KBNetworkTypeLow,
	KBNetworkTypeMedium,
	KBNetworkTypeHigh,
	KBNetworkTypeHighest,
};

#endif

@interface KBNetworkConfiguration : NSObject

@property (nonatomic, assign) NSTimeInterval bufferSize;
@property (nonatomic, assign) KBNetworkType status;

- (instancetype _Nonnull) initWithStatus: (KBNetworkType) status andBufferSize: (NSTimeInterval) size;

@end
