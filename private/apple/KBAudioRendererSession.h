//
//  KBAudioRendererSession.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 27/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBSession.h"

#import "../../private/apple/KBAudioRendererSessionDelegate.h"

#if defined __cplusplus
	#include <vector>
	#import "../../private/network_performance_metrics.h"
#endif

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>
#import <AudioToolbox/AudioToolbox.h>

@interface KBAudioRendererSession : NSObject <KBNetworkWatcher>

@property (nonatomic, assign, getter=isRunning) BOOL running;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;
@property (nonatomic, assign) AudioStreamBasicDescription audioFormat;
@property (nonatomic, weak, nullable) id <KBAudioRendererSessionDelegate> delegate;

- (instancetype _Nonnull) initWithRtmpSettings:(KBRtmpSessionSettings *_Nonnull) settings;

- (void) start;
- (void) startIfNeeded;
- (void) end;
- (void) reset;

#if defined __cplusplus
- (void) appendAudioData: (std::vector<uint8_t>) data withPresentationTime: (CMTime) presentationTime;
- (void) networkTypeChanged: (KBNetworkType) networkType;
#endif

@end
