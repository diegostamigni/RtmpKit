# RtmpKit Ltd
# Diego Stamigni

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(deps_DIR ${CMAKE_SOURCE_DIR}/libs/android)

set (SOURCES
        ${SOURCES}
        ${JAVA_BRIDGES})

set(LIB_MODE SHARED)

add_library(lib_boost_system STATIC IMPORTED)
set_target_properties(lib_boost_system PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_system.a)

add_library(lib_boost_atomic STATIC IMPORTED)
set_target_properties(lib_boost_atomic PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_atomic.a)

add_library(lib_boost_chrono STATIC IMPORTED)
set_target_properties(lib_boost_chrono PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_chrono.a)

add_library(lib_boost_date_time STATIC IMPORTED)
set_target_properties(lib_boost_date_time PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_date_time.a)

add_library(lib_boost_filesystem STATIC IMPORTED)
set_target_properties(lib_boost_filesystem PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_filesystem.a)

add_library(lib_boost_log STATIC IMPORTED)
set_target_properties(lib_boost_log PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_log.a)

add_library(lib_boost_log_setup STATIC IMPORTED)
set_target_properties(lib_boost_log_setup PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_log_setup.a)

add_library(lib_boost_regex STATIC IMPORTED)
set_target_properties(lib_boost_regex PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_regex.a)

add_library(lib_boost_thread STATIC IMPORTED)
set_target_properties(lib_boost_thread PROPERTIES IMPORTED_LOCATION
	${deps_DIR}/boost/lib/${ANDROID_ABI}/libboost_thread.a)

include(Library.cmake)

# include_directories(${deps_DIR}/boost/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${deps_DIR}/boost/include)

target_link_libraries(${PROJECT_NAME} 
	log 
	android
	lib_boost_system
	lib_boost_atomic
	lib_boost_chrono
	lib_boost_date_time
	lib_boost_filesystem
	lib_boost_log
	lib_boost_log_setup
	lib_boost_regex
	lib_boost_thread
)
