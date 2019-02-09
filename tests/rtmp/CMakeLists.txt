#
#  CMakeLists.txt
#  RtmpKit
#
#  Created by Diego Stamigni on 22/09/2016.
#  Copyright © 2016 Diego Stamigni. All rights reserved.
#

cmake_minimum_required(VERSION 3.0)

project(rtmp-tests)

if (UNIX)
    if (APPLE)
        add_compile_options(-std=c++14 -stdlib=libc++)
    else ()
        add_compile_options(-std=c++11)
    endif ()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    add_compile_options(-Wno-unused-parameter -Wno-unused-function)
elseif (WIN32)
	add_compile_options(-D_WIN32_WINDOWS)
	add_compile_options(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
	add_compile_options(-D_SCL_SECURE_NO_WARNINGS)
else ()
    message(FATAL_ERROR "unsupported platform")
endif ()

# conan (osx: brew install conan)
# run `conan install' before running cmake
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/../conanbuildinfo.cmake")
    if (APPLE)
        message(FATAL_ERROR "`conan' is not present in your system. Please install it using homebrew (brew install conan), then run `conan install' here in ${PROJECT_SOURCE_DIR}/../")
    else()
        message(FATAL_ERROR "`conan' is not present in your system. Please install it, then run `conan install', then run `conan install' here in ${PROJECT_SOURCE_DIR}/../")
    endif()
endif()

include(../conanbuildinfo.cmake)
conan_basic_setup()

set(SOURCE_FILES
        ../utils.h
        socket_test.cc
        rtmp_watcher_test.cc
        rtmp_broadcaster_test.cc)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../../include)
target_link_libraries(${PROJECT_NAME} rtmpkit ${CONAN_LIBS})