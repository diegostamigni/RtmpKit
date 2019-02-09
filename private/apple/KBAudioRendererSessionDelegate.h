//
//  KBAudioRendererSessionDelegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 28/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>
#import <AudioToolbox/AudioToolbox.h>

@protocol KBAudioRendererSessionDelegate <NSObject>

- (void) renderingAudioSampleAtPresentationTime: (CMTime) presentationTime;
- (BOOL) canRenderAudioSampleWithPresentationTime: (CMTime) presentationTime;

@end
