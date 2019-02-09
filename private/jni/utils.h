//
//  JNI/utils.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 3/04/2017.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <jni.h>

#define JNI_METHOD_RET(x) \
    extern "C" JNIEXPORT x JNICALL

JNIEnv* getJNIEnv(JavaVM* jvm);

void callVoidMethod(JNIEnv* env, jobject object, const char* name);

void callVoidMethod(JNIEnv* env, jclass clazz, const char* name);

jclass getRtmpSettingsClass();
