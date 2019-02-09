//
//  KBAudioCaptureSessionDelegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 18/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

@protocol KBAudioCaptureSessionDelegate <NSObject>

- (void) audioSampleReceived: (const uint8_t * _Nullable) data
				  withLenght: (size_t) lenght
		withPresentationTime: (uint32_t) timestamp
				 andDuration: (CMTime) duration;

@end
