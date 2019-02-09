//
//  KBAVAudioSessionManager.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 6/12/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@class KBAVAudioSessionManager;
@protocol KBAVAudioSessionManagerDelegate

@required
- (void) audioSessionManager: (KBAVAudioSessionManager *_Nonnull) audioManager
	  audioInterruptionBegan: (AVAudioSessionInterruptionType) type;

@required
- (void) audioSessionManager: (KBAVAudioSessionManager *_Nonnull) audioManager
	  audioInterruptionEnded: (AVAudioSessionInterruptionType) type;

@required
- (void) audioSessionManager: (KBAVAudioSessionManager *_Nonnull) audioManager
audioInterruptionShouldResume: (AVAudioSessionInterruptionOptions) type;

@end

@interface KBAVAudioSessionManager : NSObject
@property (nonatomic, weak, nullable) id<KBAVAudioSessionManagerDelegate> delegate;

+ (instancetype _Nonnull) sharedInstance;

- (void) setActive:(BOOL) active;
- (BOOL) isActive;
- (BOOL) isRecording;

- (void) prepareForRecordingIfNeeded;
- (void) prepareForPlaybackIfNeeded;

@end
