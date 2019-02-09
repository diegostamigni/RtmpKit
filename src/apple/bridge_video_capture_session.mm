#import "../../private/apple/KBVideoCaptureSession.h"
#include "../../private/logger.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
#endif

#import <CoreImage/CoreImage.h>
#import <CoreMedia/CoreMedia.h>

@interface KBVideoCaptureSession() <AVCaptureVideoDataOutputSampleBufferDelegate>
{
}

@property (nonatomic, assign) BOOL permissionGranted;
@property (nonatomic, assign) BOOL useFrontCamera;
@property (nonatomic, strong) AVCaptureSession *session;

@property (nonatomic, strong) AVCaptureDevice *camera;
@property (nonatomic, strong) AVCaptureDeviceInput *cameraInput;

@property (nonatomic, strong) AVCaptureVideoDataOutput *cameraOutput;

@property (nonatomic, strong) dispatch_queue_t captureVideoQueue;
@property (nonatomic, strong) KBVideoCompressorSession *compressionSession;
@property (nonatomic, strong) AVCaptureConnection *cameraConnection;
@property (nonatomic, assign) bool running;

@end

@implementation KBVideoCaptureSession
@synthesize delegate = _delegate;

- (instancetype)init
{
	self = [super init];
	
	if (self)
	{
		[self setup:false];
	}
	
	return self;
}

- (instancetype)initUsingFrontCamera:(BOOL)useFrontCamera
{
	self = [super init];
	
	if (self)
	{
		[self setup:useFrontCamera];
	}
	
	return self;
}

- (void) startIfNeeded
{
	if (![self.session isRunning] || [self.session isInterrupted])
	{
		[self start];
	}
}

- (void) detachObserver
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) attachObserver
{
#if defined TARGET_IOS
	[self detachObserver];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(orientationChanged:)
												 name:UIApplicationDidChangeStatusBarOrientationNotification
											   object:nil];
#endif
}

- (void)start
{
	if (!self.permissionGranted) return;
	
	if (!self.compressionSession)
	{
		[self initCompressor];
	}
	
	self.running = true;
	[self fixCameraOrientation];
	[self.session startRunning];
}

- (void) end
{
	self.running = false;
	[self.compressionSession end];
	[self detachObserver];
}

- (void) setup:(BOOL) useFrontCamera
{
	[self attachObserver];
	
	self.session = [AVCaptureSession new];
	self.useFrontCamera = !useFrontCamera; // the value will be inverted in the switch camera method
	
	[self addCameraDataOutput];
	[self addCameraViewOutput];
	[self switchCamera];
	
	[self.session startRunning];
}

- (void) addCameraDataOutput
{
	[self.session beginConfiguration];
	
	const auto hpAttr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, -1);
	self.captureVideoQueue = dispatch_queue_create("CaptureVideoSessionQueue", hpAttr);
	
	self.cameraOutput = [AVCaptureVideoDataOutput new];
	[self.cameraOutput setSampleBufferDelegate: self queue: self.captureVideoQueue];
	
	if ([self.session canAddOutput:self.cameraOutput])
	{
		[self.session addOutput:self.cameraOutput];
	}
	else
	{
		LOG_DEBUG("Unable to add the camera output");
	}
	
	[self.session commitConfiguration];
}

- (void) addCameraViewOutput
{
	self.layerOutput = [AVCaptureVideoPreviewLayer layerWithSession:self.session];
}

- (void) initCompressor
{
	self.compressionSession = [[KBVideoCompressorSession alloc] initWithCodecType: kCMVideoCodecType_H264
																  andRtmpSettings: self.rtmpSettings];
}

- (void) switchCamera
{
	self.useFrontCamera = !self.useFrontCamera;
	
	[self checkPermissionWithCompletionHandler:^(BOOL granted)
	{
		if (!granted)
		{
			LOG_DEBUG("Camera permissions not granted");
			return;
		}
		
		self.permissionGranted = granted;
		AVCaptureDevice *inputDevice = nil;
		NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
		
		for (AVCaptureDevice *camera in devices)
		{
			if (self.useFrontCamera)
			{
				if ([camera position] == AVCaptureDevicePositionFront)
				{
					inputDevice = camera;
					break;
				}
			}
			else
			{
				if ([camera position] != AVCaptureDevicePositionFront)
				{
					inputDevice = camera;
					break;
				}
			}
		}
		
		if (!inputDevice)
		{
			LOG_DEBUG("requested inputDevice is invalid");
			return;
		}
		else
		{
			self.camera = inputDevice;
		}
		
		[self.session beginConfiguration];
		
		if (self.cameraInput)
		{
			[self.session removeInput:self.cameraInput];
		}
		
		NSError *error;
		AVCaptureDeviceInput *cameraInput = [[AVCaptureDeviceInput alloc] initWithDevice:self.camera error: &error];
		
		if (!error && [self.session canAddInput: cameraInput])
		{
			self.cameraInput = cameraInput;
			[self.session addInput:self.cameraInput];
			self.cameraConnection = [self.cameraOutput connectionWithMediaType:AVMediaTypeVideo];
		}
		else
		{
			if (error)
			{
                LOG_DEBUG(boost::format("Unable to add the video AVCaptureDeviceInput: %1%") % [error debugDescription]);
			}
			else
			{
				LOG_DEBUG("Unable to add the video AVCaptureDeviceInput");
			}
			
			return;
		}
		
		[self fixCameraOrientation];
		[self updateCameraConfiguration];
		[self.session setSessionPreset:[self sessionPreset]];
		[self.session commitConfiguration];
	}];
}

- (void) updateCameraConfiguration
{
	if (!self.camera) return;
	
	NSError *error;
	BOOL canChangeConfiguration = [self.camera lockForConfiguration:&error];
	
	if (!error && canChangeConfiguration)
	{
		AVCaptureDeviceFormat *format = [self formatCameraWithPixelFormatType: [self defaultVideoFormatType]
																andDimensions: [self defaultVideoDimension]];
		if (format)
		{
			[self.camera setActiveFormat:format];
		}
		
		if ([self.camera isLowLightBoostSupported])
		{
			[self.camera setAutomaticallyEnablesLowLightBoostWhenAvailable:true];
		}
		
		if ([self.camera isSmoothAutoFocusSupported])
		{
			[self.camera setSmoothAutoFocusEnabled:true];
		}
		
		if ([self.camera isAutoFocusRangeRestrictionSupported])
		{
			[self.camera setAutoFocusRangeRestriction: AVCaptureAutoFocusRangeRestrictionNone];
		}
		
		if (self.cameraConnection && [self.cameraConnection isVideoStabilizationSupported])
		{
			// AVCaptureVideoStabilizationModeCinematic is causing timestamp delay
			[self.cameraConnection setPreferredVideoStabilizationMode:AVCaptureVideoStabilizationModeStandard];
		}
		
		[self.camera unlockForConfiguration];
	}
}

- (AVCaptureDeviceFormat *) formatCameraWithPixelFormatType: (CMMediaType) type
											  andDimensions: (CMVideoDimensions) dimensions
{
	if (!self.camera) return nil;
	
	for (AVCaptureDeviceFormat *format in self.camera.formats)
	{
		CMFormatDescriptionRef description = format.formatDescription;
		CMVideoDimensions currentDimensions = CMVideoFormatDescriptionGetDimensions(description);
		CMMediaType currentMediaType = CMFormatDescriptionGetMediaType(description);
		
		if ((currentDimensions.width == dimensions.width && currentDimensions.height == dimensions.height)
			&& currentMediaType == type)
		{
			for (AVFrameRateRange *range in format.videoSupportedFrameRateRanges)
			{
				if (range.maxFrameRate == [self defaultMaxFrameRate])
				{
					return format;
				}
			}
		}
	}
	
	return nil;
}

- (void)reset
{
	[self.compressionSession reset];
}

- (id<KBVideoCompressorSessionDelegate>)delegate
{
	return _delegate;
}

- (void)setDelegate:(id<KBVideoCompressorSessionDelegate>)delegate
{
	_delegate = delegate;
	self.compressionSession.delegate = delegate;
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
	
	[self.compressionSession setRtmpSettings:rtmpSettings];
}

- (KBVideoCompressorSession *)videoCompressorSession
{
	return self.compressionSession;
}

#pragma mark - AVCaptureVideoDataOutputSampleBufferDelegate

- (void) captureOutput:(AVCaptureOutput *)captureOutput
 didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
		fromConnection:(AVCaptureConnection *)connection
{
	@autoreleasepool
	{
		if (!self.running) return;
		[self.compressionSession processSampleBuffer:sampleBuffer];
	}
}

#pragma mark - Utility methods

#if defined TARGET_IOS

- (void) orientationChanged: (NSNotification *_Nullable) notification
{
	[self fixCameraOrientation];
}

- (void) fixCameraOrientation
{
	UIInterfaceOrientation currentCameraOrientation = [[UIApplication sharedApplication] statusBarOrientation];
	
	switch (currentCameraOrientation)
	{
		case UIInterfaceOrientationLandscapeLeft:
		{
			[self switchCameraOrientationToConfiguration:AVCaptureVideoOrientationLandscapeLeft];
			break;
		}
			
		case UIInterfaceOrientationLandscapeRight:
		{
			[self switchCameraOrientationToConfiguration:AVCaptureVideoOrientationLandscapeRight];
			break;
		}
			
		case UIInterfaceOrientationPortrait:
		{
			[self switchCameraOrientationToConfiguration:AVCaptureVideoOrientationPortrait];
			break;
		}
			
		case UIInterfaceOrientationPortraitUpsideDown:
		{
			[self switchCameraOrientationToConfiguration:AVCaptureVideoOrientationPortraitUpsideDown];
			break;
		}
			
		default:
			break;
	}
}

#endif

- (void) switchCameraOrientationToConfiguration: (AVCaptureVideoOrientation) orientation
{
	if (!self.cameraOutput)
	{
		return;
	}
	
	AVCaptureConnection *connection = [self.cameraOutput connectionWithMediaType:AVMediaTypeVideo];
	
	if (connection && [connection isVideoOrientationSupported])
	{
		[connection setVideoOrientation:orientation];
	}
	
	if (self.layerOutput && self.layerOutput.connection && [self.layerOutput.connection isVideoOrientationSupported])
	{
		[self.layerOutput.connection setVideoOrientation:orientation];
	}
}

- (NSString *) sessionPreset
{
	if ([self.session canSetSessionPreset:AVCaptureSessionPreset1280x720])
	{
		return AVCaptureSessionPreset1280x720;
	}
	
	return AVCaptureSessionPresetHigh;
}

- (void) checkPermissionWithCompletionHandler: (void (^)(BOOL granted)) handler
{
	AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
	
	switch (status)
	{
		case AVAuthorizationStatusNotDetermined:
		{
			[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:handler];
			break;
		}
			
		case AVAuthorizationStatusAuthorized:
		{
			handler(true);
			break;
		}
			
		default:
		{
			handler(false);
			break;
		}
	}
}

- (CMVideoDimensions) defaultVideoDimension
{
	return CMVideoDimensions{1280, 720};
}

- (CMMediaType) defaultVideoFormatType
{
	return kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
}

- (Float64) defaultMaxFrameRate
{
	return 30.;
}

@end
