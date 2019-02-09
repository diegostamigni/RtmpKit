//
//  KBFLVReader.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "KBFLVMediaTags.h"

@protocol KBFLVReaderDelegate <NSObject>

@optional - (void) AMFPacketReceived: (KBFLVAMFMediaTag *_Nullable) tag;
@optional - (void) videoPacketReceived: (KBFLVVideoMediaTag *_Nullable) tag;
@optional - (void) audioPacketReceived: (KBFLVAudioMediaTag *_Nullable) tag;
@optional - (void) parsingCompleted;

@end

@interface KBFLVReader : NSObject
@property (nonatomic, weak) id<KBFLVReaderDelegate> _Nullable delegate;

- (instancetype _Nonnull) init;
- (BOOL)read: (NSString *_Nonnull) item;

@end
