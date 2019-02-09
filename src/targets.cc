#include "../private/targets.h"
using namespace RtmpKit;

TargetType RtmpKit::currentTargetType()
{
#if defined TARGET_MACOS
	return TargetType::macOS;
#elif defined TARGET_IOS
	return TargetType::iOS;
#elif defined TARGET_WINDOWS
		return TargetType::Windows;
#elif defined TARGET_LINUX
		return TargetType::Linux;
#elif defined TARGET_ANDROID
		return TargetType::Android;
#else
		return TargetType::Unknown;
#endif
}
