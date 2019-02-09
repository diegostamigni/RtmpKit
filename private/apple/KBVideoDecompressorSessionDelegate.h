//
//  KBVideoDecompressorSessionDelegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreImage/CoreImage.h>
#import <CoreMedia/CoreMedia.h>

@protocol KBVideoDecompressorSessionDelegate <NSObject>

- (void) videoSampleDecoded: (CIImage* _Nullable) image
	   withPresentationTime: (CMTime) presentationTime
				andDuration: (CMTime) duration
			   withTimebase: (CMTimebaseRef _Nonnull) timeBase;

@end
