LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


# OpenAL
LOCAL_MODULE := openal
LOCAL_SRC_FILES := obj/openal/libopenal.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# OGG
LOCAL_MODULE := ogg
LOCAL_SRC_FILES := obj/libogg/src/.libs/libogg.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Vorbis
LOCAL_MODULE := vorbis
LOCAL_SRC_FILES := obj/libvorbis/lib/.libs/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Vorbisfile
LOCAL_MODULE := vorbisfile
LOCAL_SRC_FILES := obj/libvorbis/lib/.libs/libvorbisfile.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# CURL
LOCAL_MODULE := curl
LOCAL_SRC_FILES := obj/curl/lib/.libs/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# libcrypto
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := obj/openssl/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# libssl
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := obj/openssl/libssl.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# JPEG
LOCAL_MODULE := jpeglib
LOCAL_SRC_FILES := obj/jpeglib/libjpeglib.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Freetype
LOCAL_MODULE := freetype
LOCAL_SRC_FILES := obj/freetype/objs/.libs/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# zlib
LOCAL_MODULE := zlib
LOCAL_SRC_FILES := obj/zlib/libz.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# PNG
LOCAL_MODULE := png
LOCAL_SRC_FILES := obj/libpng/libpng.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# ifaddrs
LOCAL_MODULE    := ifaddrs
LOCAL_PATH      := .
LOCAL_SRC_FILES := ../lib/ifaddrs/ifaddrs.c
LOCAL_CFLAGS    := -I../lib/ifaddrs
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# AngelScript
LOCAL_MODULE       := angelscript
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../lib/angelscript/source/*.S)   \
                      $(wildcard ../lib/angelscript/source/*.cpp)
LOCAL_CFLAGS       := -I../lib/angelscript/source/
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# ENET
LOCAL_MODULE       := enet
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/enet/*.c)
LOCAL_CFLAGS       := -I../lib/enet/include/ -DHAS_SOCKLEN_T
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Bullet
LOCAL_MODULE       := bullet
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/bullet/src/*/*.cpp)   \
                      $(wildcard ../lib/bullet/src/*/*/*.cpp)
LOCAL_CFLAGS       := -I../lib/bullet/src/
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Irrlicht
LOCAL_MODULE       := irrlicht
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/irrlicht/source/Irrlicht/*.cpp)        \
                      $(wildcard ../lib/irrlicht/source/Irrlicht/Android/*.cpp)
LOCAL_CFLAGS       := -I../lib/irrlicht/source/Irrlicht/ \
                      -I../lib/irrlicht/include/         \
                      -Iobj/jpeglib/                     \
                      -Iobj/libpng/                      \
                      -Iobj/zlib/                        \
                      -I$(call my-dir)/../../sources/android/native_app_glue
                      -std=gnu++0x
LOCAL_STATIC_LIBRARIES := jpeglib png zlib
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# STK
LOCAL_MODULE       := main
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../src/*.cpp)     \
                      $(wildcard ../src/*/*.cpp)   \
                      $(wildcard ../src/*/*/*.cpp)
LOCAL_LDLIBS       := -llog -landroid -lEGL -lGLESv3 -lOpenSLES
LOCAL_CFLAGS       := -I../lib/angelscript/include      \
                      -I../lib/bullet/src               \
                      -I../lib/enet/include             \
                      -I../lib/ifaddrs                  \
                      -I../lib/irrlicht/include         \
                      -I../lib/irrlicht/source/Irrlicht \
                      -I../src                          \
                      -Iobj/curl/include                \
                      -Iobj/freetype/include            \
                      -Iobj/libogg/include              \
                      -Iobj/libvorbis/include           \
                      -Iobj/openal/include              \
                      -I$(call my-dir)/../../sources/android/native_app_glue \
                      -DUSE_GLES2      \
                      -DHAVE_OGGVORBIS \
                      -DNDEBUG         \
                      -std=gnu++0x

LOCAL_STATIC_LIBRARIES := irrlicht bullet enet freetype ifaddrs angelscript  \
                          vorbisfile vorbis ogg openal curl libssl libcrypto \
                          gnustl_static android_native_app_glue

include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

$(call import-module,android/native_app_glue)
