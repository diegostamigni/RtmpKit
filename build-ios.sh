#!/bin/sh
# Diego Stamigni

BUILD_DIR="$(pwd)/build-ios"
OUTPUT_DIR="${BUILD_DIR}/../ios/output"
PROJECT_NAME="rtmpkit"
OUTPUT_NAME="lib${PROJECT_NAME}.a"
BUILD_TYPE="Release"
LIB_VERSION=""

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

COMMON_PARAMS="-DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DIOS_DEPLOYMENT_TARGET=8.1 -DLIB_VERSION=${LIB_VERSION}"

# setup deps
cd ${BUILD_DIR}/..
git submodule init 3rdparty/ios-cmake
git submodule init 3rdparty/SAMContentMode
git submodule update

CONAN_PARAMS="--build=missing"
conan install ${CONAN_PARAMS} .

cd ${BUILD_DIR}/../tests
conan install ${CONAN_PARAMS} .

# remove old stuff if found
rm -rf ${OUTPUT_DIR}
rm -rf ${BUILD_DIR}

# start building
mkdir -p ${OUTPUT_DIR}
cd ${BUILD_DIR}

cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../3rdparty/ios-cmake/toolchain/iOS.cmake -DIOS_PLATFORM=SIMULATOR ${COMMON_PARAMS}
xcodebuild OTHER_CFLAGS="-fembed-bitcode" -project ${PROJECT_NAME}.xcodeproj -target ${PROJECT_NAME} -configuration ${BUILD_TYPE}

if [ -f $(pwd)/lib/lib${PROJECT_NAME}.a ]; then
	cp $(pwd)/lib/lib${PROJECT_NAME}.a ${OUTPUT_DIR}/lib${PROJECT_NAME}-simulator.a
fi

rm -rf *
cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../3rdparty/ios-cmake/toolchain/iOS.cmake -DIOS_PLATFORM=SIMULATOR64 ${COMMON_PARAMS}
xcodebuild OTHER_CFLAGS="-fembed-bitcode" -project ${PROJECT_NAME}.xcodeproj -target ${PROJECT_NAME} -configuration ${BUILD_TYPE}

if [ -f $(pwd)/lib/lib${PROJECT_NAME}.a ]; then
	cp $(pwd)/lib/lib${PROJECT_NAME}.a ${OUTPUT_DIR}/lib${PROJECT_NAME}-simulator64.a
fi

rm -rf *
cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../3rdparty/ios-cmake/toolchain/iOS.cmake -DIOS_PLATFORM=OS ${COMMON_PARAMS}
xcodebuild OTHER_CFLAGS="-fembed-bitcode" -project ${PROJECT_NAME}.xcodeproj -target ${PROJECT_NAME} -configuration ${BUILD_TYPE}

if [ -f $(pwd)/lib/lib${PROJECT_NAME}.a ]; then
	cp $(pwd)/lib/lib${PROJECT_NAME}.a ${OUTPUT_DIR}/lib${PROJECT_NAME}-os.a
fi

cd ${OUTPUT_DIR}
rm -rf ${OUTPUT_NAME}

LIBS_TO_BE_MERGED=""

if [ -f lib${PROJECT_NAME}-simulator.a ]; then
    LIBS_TO_BE_MERGED="lib${PROJECT_NAME}-simulator.a"
fi

if [ -f lib${PROJECT_NAME}-simulator64.a ]; then
    LIBS_TO_BE_MERGED="${LIBS_TO_BE_MERGED} lib${PROJECT_NAME}-simulator64.a"
fi

if [ -f lib${PROJECT_NAME}-os.a ]; then
    LIBS_TO_BE_MERGED="${LIBS_TO_BE_MERGED} lib${PROJECT_NAME}-os.a"
fi

if [ ! -z "${LIBS_TO_BE_MERGED}" ]; then
	libtool -static -a ${LIBS_TO_BE_MERGED} -o ${OUTPUT_NAME}
	rm -rf ${LIBS_TO_BE_MERGED}
fi

rsync -avp ${BUILD_DIR}/../include .
