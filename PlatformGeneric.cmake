# RtmpKit Ltd
# Diego Stamigni

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_property(CURRENT_TARGET_TYPE GLOBAL PROPERTY CURRENT_TARGET_TYPE)
message ("Building for ${CURRENT_TARGET_TYPE}")

if (UNIX)
    if (APPLE)
        if ("${CURRENT_TARGET_TYPE}" STREQUAL "")
            set(CURRENT_TARGET_TYPE TARGET_MACOS)
        endif()

        add_compile_options(-fobjc-arc)

        set (SOURCES
                ${SOURCES}
                ${APPLE_BRIDGES})
    else ()
        set(CURRENT_TARGET_TYPE TARGET_LINUX)
    endif ()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    
    add_compile_options(-Wno-unused-parameter -Wno-unused-function -Winfinite-recursion -Wmove -Wrange-loop-analysis)
elseif (WIN32)
    if ("${CURRENT_TARGET_TYPE}" STREQUAL "")
        set(CURRENT_TARGET_TYPE TARGET_WINDOWS)
    endif()

    add_compile_options(-D_WIN32_WINDOWS)
    add_compile_options(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    add_compile_options(-D_SCL_SECURE_NO_WARNINGS)
else()
    message(FATAL_ERROR "unsupported platform")
endif ()

# conan (osx: brew install conan)
# run `conan install' before running cmake
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/conanbuildinfo.cmake")
    if (APPLE)
        message(FATAL_ERROR "`conan' is not present in your system. Please install it using homebrew (brew install conan), then run `conan install' here in ${PROJECT_SOURCE_DIR}/../")
    else()
        message(FATAL_ERROR "`conan' is not present in your system. Please install it, then run `conan install', then run `conan install' here in ${PROJECT_SOURCE_DIR}/../")
    endif()
endif()

include(conanbuildinfo.cmake)
conan_basic_setup()

# install configs
set(LIBRARY_INSTALL_DIR bin)
set(INCLUDE_INSTALL_DIR include)
set(LIB_MODE STATIC)

# tests
set(TESTS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/tests")
if(EXISTS "${TESTS_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory(${TESTS_SOURCE_DIR})
endif()

include(Library.cmake)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_DIR})
install(FILES ${SOURCE_FILES} "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_export.h" DESTINATION ${INCLUDE_INSTALL_DIR})

if ("${CURRENT_TARGET_TYPE}" STREQUAL "TARGET_IOS" AND (NOT "${IOS_DEPLOYMENT_TARGET}" STREQUAL ""))
    message ("Using IOS_DEPLOYMENT_TARGET=${IOS_DEPLOYMENT_TARGET}")
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")
endif()

macro(GroupSources curdir)
   file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)
   foreach(child ${children})
          if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
                GroupSources(${curdir}/${child})
          else()
                string(REPLACE "/" "\\" groupname ${curdir})
                source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
          endif()
   endforeach()
endmacro()

if (APPLE)
    find_program(CLANG_TIDY_BIN_PATH "clang-tidy")

    if(NOT CLANG_TIDY_BIN_PATH)
        set(CLANG_TIDY_BIN_PATH "/usr/local/opt/llvm/bin/clang-tidy")
    endif()

    if (NOT EXISTS "${CLANG_TIDY_BIN_PATH}")
    else()
       # for a ref. on what is available, check http://clang.llvm.org/extra/clang-tidy/#using-clang-tidy
       set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_CLANG_TIDY
               "${CLANG_TIDY_BIN_PATH}"
               "-checks=-*,clang-analyzer-*,cppcoreguidelines-*,boost-*,modernize-*,performance-*,misc-*")
   endif()
endif()

GroupSources(include)
GroupSources(private)
GroupSources(src)
