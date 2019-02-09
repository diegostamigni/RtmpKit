//
//  KBLibConfig.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 3/1/2017.
//  Copyright © 2017 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, KBVideoCaptureSessionViewOnStageSide) {
	KBVideoCaptureSessionViewOnStageSideLeft = 0,
	KBVideoCaptureSessionViewOnStageSideRight = 1,
};

@interface KBLibConfig : NSObject

+ (NSString *_Nonnull) version;

@end
