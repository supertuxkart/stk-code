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
LOCAL_MODULE := libjpeg
LOCAL_SRC_FILES := obj/libjpeg/libjpeg.a
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


# Fribidi
LOCAL_MODULE       := fribidi
LOCAL_SRC_FILES    := obj/fribidi/lib/.libs/libfribidi.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Freetype
LOCAL_MODULE := freetype
LOCAL_SRC_FILES := obj/freetype/objs/.libs/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Harfbuzz
LOCAL_MODULE       := harfbuzz
LOCAL_SRC_FILES    := obj/harfbuzz/src/.libs/libharfbuzz.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Raqm
LOCAL_MODULE       := raqm
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/libraqm/*.c)
LOCAL_CFLAGS       := -Iobj/fribidi/include             \
                      -Iobj/freetype/include            \
                      -Iobj/harfbuzz/include            \
                      -DHAVE_DECL_HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES \
                      -DHAVE_HB_BUFFER_SET_INVISIBLE_GLYPH \
                      -DHAVE_HB_FT_FONT_CREATE_REFERENCED \
                      -DHAVE_HB_FT_FONT_SET_LOAD_FLAGS \
                      -std=c99
include $(BUILD_STATIC_LIBRARY)
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
LOCAL_CFLAGS       := -I../lib/enet/include/ -DHAS_SOCKLEN_T -DENABLE_IPV6
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


# Graphics utils
LOCAL_MODULE       := graphics_utils
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/graphics_utils/mipmap/*.c)
LOCAL_CFLAGS       := -I../lib/graphics_utils/mipmap
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# MCPP
LOCAL_MODULE       := mcpp
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/mcpp/*.c)
LOCAL_CFLAGS       := -DMCPP_LIB -DHAVE_CONFIG_H
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Irrlicht
LOCAL_MODULE       := irrlicht
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/irrlicht/source/Irrlicht/*.cpp)
LOCAL_CFLAGS       := -I../lib/irrlicht/source/Irrlicht/ \
                      -I../lib/irrlicht/include/         \
                      -I../src                           \
                      -Iobj/libjpeg/                     \
                      -Iobj/libpng/                      \
                      -Iobj/zlib/                        \
                      -I../lib/sdl2/include/             \
                      -DMOBILE_STK                       \
                      -DANDROID_PACKAGE_CALLBACK_NAME=$(PACKAGE_CALLBACK_NAME)
LOCAL_CPPFLAGS     := -std=gnu++0x
LOCAL_STATIC_LIBRARIES := libjpeg png zlib
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

# SDL2
LOCAL_MODULE       := SDL2
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/sdl2/src/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/android/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/dummy/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/openslES/*.c) \
                      $(wildcard ../lib/sdl2/src/core/android/*.c) \
                      $(wildcard ../lib/sdl2/src/cpuinfo/*.c) \
                      $(wildcard ../lib/sdl2/src/dynapi/*.c) \
                      $(wildcard ../lib/sdl2/src/events/*.c) \
                      $(wildcard ../lib/sdl2/src/file/*.c) \
                      $(wildcard ../lib/sdl2/src/haptic/*.c) \
                      $(wildcard ../lib/sdl2/src/haptic/android/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/android/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/hidapi/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/virtual/*.c) \
                      $(wildcard ../lib/sdl2/src/locale/android/*.c) \
                      $(wildcard ../lib/sdl2/src/locale/*.c) \
                      $(wildcard ../lib/sdl2/src/loadso/dlopen/*.c) \
                      $(wildcard ../lib/sdl2/src/power/*.c) \
                      $(wildcard ../lib/sdl2/src/power/android/*.c) \
                      $(wildcard ../lib/sdl2/src/filesystem/android/*.c) \
                      $(wildcard ../lib/sdl2/src/sensor/*.c) \
                      $(wildcard ../lib/sdl2/src/sensor/android/*.c) \
                      $(wildcard ../lib/sdl2/src/render/*.c) \
                      $(wildcard ../lib/sdl2/src/render/*/*.c) \
                      $(wildcard ../lib/sdl2/src/stdlib/*.c) \
                      $(wildcard ../lib/sdl2/src/thread/*.c) \
                      $(wildcard ../lib/sdl2/src/thread/pthread/*.c) \
                      $(wildcard ../lib/sdl2/src/timer/*.c) \
                      $(wildcard ../lib/sdl2/src/timer/unix/*.c) \
                      $(wildcard ../lib/sdl2/src/video/*.c) \
                      $(wildcard ../lib/sdl2/src/video/android/*.c) \
                      $(wildcard ../lib/sdl2/src/video/yuv2rgb/*.c) \
                      ../lib/sdl2/src/atomic/SDL_atomic.c.arm \
                      ../lib/sdl2/src/atomic/SDL_spinlock.c.arm \
                      ../lib/sdl2/src/hidapi/android/hid.cpp
LOCAL_CFLAGS       := -I../lib/sdl2/include/ -DGL_GLEXT_PROTOTYPES
LOCAL_CPPFLAGS     := -std=gnu++0x
LOCAL_STATIC_LIBRARIES := cpufeatures

include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

# STK
LOCAL_MODULE       := main
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../src/*.cpp)     \
                      $(wildcard ../src/*/*.cpp)   \
                      $(wildcard ../src/*/*/*.cpp)
LOCAL_LDLIBS       := -llog -landroid -lEGL -lGLESv1_CM -lGLESv3 -lOpenSLES -ldl -lm
LOCAL_CFLAGS       := -I../lib/angelscript/include      \
                      -I../lib/bullet/src               \
                      -I../lib/libraqm                  \
                      -I../lib/enet/include             \
                      -I../lib/ifaddrs                  \
                      -I../lib/irrlicht/include         \
                      -I../lib/irrlicht/source/Irrlicht \
                      -I../lib/graphics_utils           \
                      -I../lib/mcpp                     \
                      -I../lib/sdl2/include             \
                      -I../src                          \
                      -Iobj/curl/include                \
                      -Iobj/fribidi/include             \
                      -Iobj/freetype/include            \
                      -Iobj/harfbuzz/include            \
                      -Iobj/libogg/include              \
                      -Iobj/libvorbis/include           \
                      -Iobj/openal/include              \
                      -Iobj/openssl/include             \
                      -DUSE_GLES2      \
                      -DMOBILE_STK     \
                      -DENABLE_SOUND   \
                      -DENABLE_IPV6    \
                      -DENABLE_CRYPTO_OPENSSL           \
                      -DNDEBUG         \
                      -DANDROID_PACKAGE_NAME=\"$(PACKAGE_NAME)\"    \
                      -DANDROID_APP_DIR_NAME=\"$(APP_DIR_NAME)\"    \
                      -DSUPERTUXKART_VERSION=\"$(PROJECT_VERSION)\" \
                      -DANDROID_PACKAGE_CALLBACK_NAME=$(PACKAGE_CALLBACK_NAME)
LOCAL_CPPFLAGS     := -std=gnu++0x

LOCAL_STATIC_LIBRARIES := irrlicht bullet enet ifaddrs angelscript mcpp SDL2 \
                          vorbisfile vorbis ogg openal curl libssl libcrypto \
                          c++_static raqm fribidi harfbuzz freetype graphics_utils

include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)
$(call import-module, android/cpufeatures)
