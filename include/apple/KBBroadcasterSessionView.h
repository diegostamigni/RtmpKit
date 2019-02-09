//
//  KBBroadcasterSessionView.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 18/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "KBRtmpBroadcasterSessionView.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
	@interface KBBroadcasterSessionView : KBRtmpBroadcasterSessionView
#else
	#import <AppKit/AppKit.h>
	@interface KBBroadcasterSessionView : KBRtmpBroadcasterSessionView
#endif

@end
