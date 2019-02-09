//
//  KBAudioCaptureSession.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 18/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import "../../include/apple/KBRtmpSessionSettings.h"

#import "../../private/apple/KBAudioCaptureSessionDelegate.h"

#import <Foundation/Foundation.h>

@interface KBAudioCaptureSession : NSObject

@property (nonatomic, weak, nullable) id <KBAudioCaptureSessionDelegate> delegate;
@property (nonatomic, strong, nonnull) KBRtmpSessionSettings *rtmpSettings;

- (instancetype _Nonnull) initWithRtmpSettings:(KBRtmpSessionSettings *_Nonnull) settings;

- (void) startIfNeeded;
- (void) reset;
- (void) start;
- (void) end;

@end
