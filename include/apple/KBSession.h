//
//  KBSession.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 6/02/2017.
//  Copyright © 2017 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "KBRtmpSessionSettings.h"

@protocol KBNetworkWatcher

#if defined __cplusplus
- (void) networkTypeChanged: (KBNetworkType) networkType;
#endif

@end

@protocol KBSession

- (void) end;
- (void) realign;

@end

@protocol KBReconnectableSession

- (void) reconnectAfter:(NSInteger) waitingTimeInSeconds;

@end

@protocol KBRtmpSession <KBSession>

- (void)startWithHost: (NSString *_Nonnull) host
			  andPort: (NSString *_Nonnull) port
		   andAppName: (NSString *_Nonnull) appName
		 andStreamURL: (NSString *_Nonnull) streamURL
	 layoutForOnStage: (BOOL) onStage;

@end

@protocol KBRtmpBroadcastSession <KBRtmpSession>

- (void) switchCamera;

@end

@protocol KBRtmpWatcherSession <KBRtmpSession>

@end
