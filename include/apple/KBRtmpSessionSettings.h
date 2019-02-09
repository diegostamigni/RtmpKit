//
//  KBRtmpSessionSettings.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 14/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "KBNetworkConfiguration.h"

@interface KBRtmpSessionSettings : NSObject

@property (nonatomic, assign) double width;
@property (nonatomic, assign) double height;
@property (nonatomic, assign) double frameRate;
@property (nonatomic, assign) double audioChannels;
@property (nonatomic, assign) double videoDataRate;
@property (nonatomic, assign) double keyFrameInterval;
@property (nonatomic, assign, getter=isDynamicVideoBitRateEnabled) bool enableDynamicVideoBitRate;
@property (nonatomic, strong, nullable) NSSet<KBNetworkConfiguration*> *networkProperties;

- (KBNetworkConfiguration *_Nullable) lowestNetworkConfiguration;
- (KBNetworkConfiguration *_Nullable) highestNetworkConfiguration;
- (KBNetworkConfiguration *_Nullable) networkConfigurationWithNetworkType: (KBNetworkType) networkType;

@end
