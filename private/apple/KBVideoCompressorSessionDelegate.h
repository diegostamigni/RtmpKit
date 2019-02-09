//
//  KBVideoCompressorSessionDelegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 13/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

@protocol KBVideoCompressorSessionDelegate <NSObject>

- (void) videoSampleEncoded: (CMSampleBufferRef  _Nullable) compressedSample
	   withPresentationTime: (CMTime) presentationTime
				andDuration: (CMTime) duration;

@end
