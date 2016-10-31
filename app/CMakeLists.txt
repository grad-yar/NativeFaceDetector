# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

set(OPENCV_SDK_PATH D:/Android/OpenCV-3.1.0-android-sdk)

include_directories(${OPENCV_SDK_PATH}/sdk/native/jni/include)

add_library(detection_based_tracker SHARED ${CMAKE_SOURCE_DIR}/src/main/jni/DetectionBasedTracker_jni.cpp)

add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../opencv/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library(log-lib log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(detection_based_tracker ${log-lib} lib_opencv)