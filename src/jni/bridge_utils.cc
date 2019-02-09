#include "../../private/jni/utils.h"
#include "../../include/version.h"
#include "../../private/logger.h"

jclass rtmpSettingsClass = nullptr;

JNIEnv* getJNIEnv(JavaVM* jvm)
{
	JNIEnv *env;
	const auto status = jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

	switch (status)
	{
		case JNI_EDETACHED:
		{
			const auto rt = jvm->AttachCurrentThread(&env, NULL);
			assert (rt == JNI_OK);
			break;
		}

		default:
		{
			break;
		}
	}

	return env;
}

void callVoidMethod(JNIEnv* env, jobject object, const char* name)
{
	if (const auto clazz = env->GetObjectClass(object))
	{
		if (const auto methodID = env->GetMethodID(clazz, name, "()V"))
		{
			env->CallVoidMethod(object, methodID);
		}
	}
}

void callVoidMethod(JNIEnv* env, jclass clazz, const char* name)
{
	if (const auto methodID = env->GetMethodID(clazz, name, "()V"))
	{
		if (const auto object = env->NewObject(clazz, methodID))
		{
			env->CallVoidMethod(object, methodID);
		}
	}
}

// jclass ref. cache

jclass getRtmpSettingsClass()
{
	return rtmpSettingsClass;
}

// JNI load/unload events

JNI_METHOD_RET(jint)
JNI_OnLoad(JavaVM *pjvm, void *reserved)
{
	LOG_DEBUG("JNI_OnLoad(...)");

	if (const auto env = getJNIEnv(pjvm))
	{
		const auto clazz = env->FindClass("tv/diegostamigni/rtmpkit/rtmp/RtmpSettings");
		rtmpSettingsClass = static_cast<jclass>(env->NewGlobalRef(clazz));
	}

	return JNI_VERSION_1_6;
}

JNI_METHOD_RET(void)
JNI_OnUnload(JavaVM *vm, void *reserved)
{
	LOG_DEBUG("JNI_OnUnload(...)");

	if (const auto env = getJNIEnv(vm))
	{
		if (rtmpSettingsClass != nullptr)
		{
			env->DeleteGlobalRef(rtmpSettingsClass);
		}
	}
}
