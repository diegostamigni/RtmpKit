#!/bin/sh
# Diego Stamigni
# https://developer.android.com/ndk/guides/cmake.html

# The following env variables are required:
# ANDROID_NDK_ROOT=
# ANDROID_SDK_ROOT=
# ANDROID_CMAKE_BINARY_DIR=
# ANDROID_CMAKE_TOOLCHAIN_FILE=

BUILD_DIR="$(pwd)/build-android"
OUTPUT_DIR="${BUILD_DIR}/../android/output"
PROJECT_NAME="rtmpkit"
OUTPUT_NAME="lib${PROJECT_NAME}.a"
BUILD_TYPE="Release"
LIB_VERSION=""
ANDROID_ABI="armeabi-v7a with NEON"
ANDROID_NATIVE_API_LEVEL=15
ANDROID_TOOLCHAIN="clang"

cd ${BUILD_DIR}/..

# setup deps
git submodule init libs/android/boost
git submodule update

if [ ! -d libs/android/boost ]; then
	echo "Boost not found, failing..."
	exit 1
fi

if [ -z "$1" ]; then
	echo "$PROJECT_NAME version is unknown.."
else
	LIB_VERSION=$1
	echo "Building $PROJECT_NAME version $LIB_VERSION.."
fi

if [ -z "$2" ]; then
    	echo "BUILD_TYPE not set: using the default: $BUILD_TYPE"
else
	BUILD_TYPE="$2"
	echo "Building in $BUILD_TYPE mode.."
fi

# remove old stuff if found
rm -rf ${OUTPUT_DIR}
rm -rf ${BUILD_DIR}

# start building
mkdir -p ${OUTPUT_DIR}
cd ${BUILD_DIR}

${ANDROID_CMAKE_BINARY_DIR}/cmake .. \
	-DCMAKE_TOOLCHAIN_FILE="${ANDROID_CMAKE_TOOLCHAIN_FILE}" \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-DLIB_VERSION="${LIB_VERSION}" \
	-DANDROID_NDK="${ANDROID_NDK_ROOT}" \
	-DANDROID_ABI="${ANDROID_ABI}" \
	-DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL} \
	-DANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}

# -G"Android Gradle - Ninja"

${ANDROID_CMAKE_BINARY_DIR}/cmake --build .
cp $(pwd)/lib${PROJECT_NAME}.so ${OUTPUT_DIR}/lib${PROJECT_NAME}.so

cd ${OUTPUT_DIR}
rsync -avp ${BUILD_DIR}/../include .
