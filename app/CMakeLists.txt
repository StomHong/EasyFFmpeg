# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#set(lib_src_DIR ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

#第三方库使用到的头文件
include_directories( ${CMAKE_SOURCE_DIR}/src/main/cpp/include/)

#生成lib名称(可以随便起)
add_library( # Sets the name of the library.
             simplest_ffmpeg_streamer

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/simplest_ffmpeg_streamer.c )

#作为支持库
add_library( avcodec-56 SHARED IMPORTED )
add_library( avdevice-56 SHARED IMPORTED )
add_library( avfilter-5 SHARED IMPORTED )
add_library( avformat-56 SHARED IMPORTED )
add_library( avutil-54 SHARED IMPORTED )
add_library( postproc-53 SHARED IMPORTED )
add_library( swresample-1 SHARED IMPORTED )
add_library( swscale-3 SHARED IMPORTED )

set_target_properties(
    avcodec-56
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/libs/armeabi/libavcodec-56.so
    )

set_target_properties(
        avfilter-5
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/armeabi/libavfilter-5.so
        )

set_target_properties(
            avformat-56
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/libs/armeabi/libavformat-56.so
            )

set_target_properties(
            avutil-54
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/libs/armeabi/libavutil-54.so
            )

set_target_properties(
            swresample-1
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/libs/armeabi/libswresample-1.so
             )

set_target_properties(
            swscale-3
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/libs/armeabi/libswscale-3.so
             )


set_target_properties(
            postproc-53
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/libs/armeabi/libpostproc-53.so
             )

set_target_properties(
            avdevice-56
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/libs/armeabi/libavdevice-56.so
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                         simplest_ffmpeg_streamer
                         avdevice-56
                         avcodec-56
                         avfilter-5
                         avformat-56
                         avutil-54
                         swresample-1
                         swscale-3
                         postproc-53

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )