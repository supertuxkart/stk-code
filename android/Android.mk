LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


# OpenAL
LOCAL_MODULE := openal
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/openal/libopenal.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# OGG
LOCAL_MODULE := ogg
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/libogg/libogg.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Vorbis
LOCAL_MODULE := vorbis
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/libvorbis/lib/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Vorbisfile
LOCAL_MODULE := vorbisfile
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/libvorbis/lib/libvorbisfile.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# CURL
LOCAL_MODULE := curl
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/curl/lib/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# libmbedtls
LOCAL_MODULE := libmbedtls
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/mbedtls/library/libmbedtls.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# libmbedcrypto
LOCAL_MODULE := libmbedcrypto
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/mbedtls/library/libmbedcrypto.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# libmbedx509
LOCAL_MODULE := libmbedx509
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/mbedtls/library/libmbedx509.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# JPEG
LOCAL_MODULE := libjpeg
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/libjpeg/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# zlib
LOCAL_MODULE := zlib
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/zlib/libz.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# PNG
LOCAL_MODULE := png
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/libpng/libpng.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Freetype
LOCAL_MODULE := freetype
LOCAL_SRC_FILES := deps-$(TARGET_ARCH_ABI)/freetype/build/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Harfbuzz
LOCAL_MODULE       := harfbuzz
LOCAL_SRC_FILES    := deps-$(TARGET_ARCH_ABI)/harfbuzz/build/libharfbuzz.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# shaderc
LOCAL_MODULE       := shaderc
LOCAL_SRC_FILES    := deps-$(TARGET_ARCH_ABI)/shaderc/libshaderc/libshaderc_combined.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# libsquish
LOCAL_MODULE       := libsquish
LOCAL_SRC_FILES    := deps-$(TARGET_ARCH_ABI)/libsquish/libsquish.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# astc-encoder
LOCAL_MODULE       := libastcenc
LOCAL_SRC_FILES    := deps-$(TARGET_ARCH_ABI)/astc-encoder/Source/libastcenc.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)


# ifaddrs
LOCAL_MODULE    := ifaddrs
LOCAL_PATH      := .
LOCAL_SRC_FILES := ../lib/ifaddrs/ifaddrs.c
LOCAL_CFLAGS    := -I../lib/ifaddrs
# Starting NDK21 it enables NEON by default on 32-bit ARM target
# Disable it to support more phones
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON  := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# AngelScript
LOCAL_MODULE       := angelscript
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../lib/angelscript/source/*.S)   \
                      $(wildcard ../lib/angelscript/source/*.cpp)
LOCAL_CFLAGS       := -I../lib/angelscript/source/
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# ENET
LOCAL_MODULE       := enet
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/enet/*.c)
LOCAL_CFLAGS       := -I../lib/enet/include/ -DHAS_SOCKLEN_T -DENABLE_IPV6
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Bullet
LOCAL_MODULE       := bullet
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/bullet/src/*/*.cpp)   \
                      $(wildcard ../lib/bullet/src/*/*/*.cpp)
LOCAL_CFLAGS       := -I../lib/bullet/src/
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Graphics utils
LOCAL_MODULE       := graphics_utils
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/graphics_utils/mipmap/*.c)
LOCAL_CFLAGS       := -I../lib/graphics_utils/mipmap \
                      -I../lib/simd_wrapper
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Graphics engine
LOCAL_MODULE       := graphics_engine
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../lib/graphics_engine/src/*.c) \
                      $(wildcard ../lib/graphics_engine/src/*.cpp)
LOCAL_CFLAGS       := -DENABLE_LIBASTCENC                 \
                      -I../lib/graphics_engine/include    \
                      -I../lib/graphics_utils             \
                      -I../lib/sdl2/include/              \
                      -I../lib/bullet/src/                \
                      -I../lib/irrlicht/include/          \
                      -I../lib/shaderc/libshaderc/include \
                      -I../lib/libsquish                  \
                      -Ideps-$(TARGET_ARCH_ABI)/astc-encoder/Source
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
LOCAL_STATIC_LIBRARIES := shaderc libsquish libastcenc
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# MCPP
LOCAL_MODULE       := mcpp
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/mcpp/*.c)
LOCAL_CFLAGS       := -DMCPP_LIB -DHAVE_CONFIG_H
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# SheenBidi
LOCAL_MODULE       := sheenbidi
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES    := $(wildcard ../lib/sheenbidi/Source/*.c)
LOCAL_CFLAGS       := -I../lib/sheenbidi/Headers
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# tinygettext
LOCAL_MODULE       := tinygettext
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../lib/tinygettext/src/*.cpp)
LOCAL_CFLAGS       := -I../lib/tinygettext/include -DDISABLE_ICONV
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# Irrlicht
LOCAL_MODULE       := irrlicht
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../lib/irrlicht/source/Irrlicht/*.cpp)
LOCAL_CFLAGS       := -I../lib/irrlicht/source/Irrlicht/ \
                      -I../lib/irrlicht/include/         \
                      -I../src                           \
                      -Ideps-$(TARGET_ARCH_ABI)/libjpeg/ \
                      -Ideps-$(TARGET_ARCH_ABI)/libpng/  \
                      -Ideps-$(TARGET_ARCH_ABI)/zlib/    \
                      -I../lib/sdl2/include/             \
                      -I../lib/graphics_engine/include   \
                      -DMOBILE_STK                       \
                      -DANDROID_PACKAGE_CALLBACK_NAME=$(PACKAGE_CALLBACK_NAME)
LOCAL_CPPFLAGS     := -std=gnu++0x
LOCAL_STATIC_LIBRARIES := libjpeg png zlib
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)


# SDL2
LOCAL_MODULE       := SDL2
LOCAL_PATH         := .
LOCAL_SRC_FILES    := $(wildcard ../lib/sdl2/src/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/android/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/dummy/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/aaudio/*.c) \
                      $(wildcard ../lib/sdl2/src/audio/openslES/*.c) \
                      ../lib/sdl2/src/atomic/SDL_atomic.c.arm \
                      ../lib/sdl2/src/atomic/SDL_spinlock.c.arm \
                      $(wildcard ../lib/sdl2/src/core/android/*.c) \
                      $(wildcard ../lib/sdl2/src/cpuinfo/*.c) \
                      $(wildcard ../lib/sdl2/src/dynapi/*.c) \
                      $(wildcard ../lib/sdl2/src/events/*.c) \
                      $(wildcard ../lib/sdl2/src/file/*.c) \
                      $(wildcard ../lib/sdl2/src/haptic/*.c) \
                      $(wildcard ../lib/sdl2/src/haptic/android/*.c) \
                      $(wildcard ../lib/sdl2/src/hidapi/*.c) \
                      $(wildcard ../lib/sdl2/src/hidapi/android/*.cpp) \
                      $(wildcard ../lib/sdl2/src/joystick/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/android/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/hidapi/*.c) \
                      $(wildcard ../lib/sdl2/src/joystick/virtual/*.c) \
                      $(wildcard ../lib/sdl2/src/loadso/dlopen/*.c) \
                      $(wildcard ../lib/sdl2/src/locale/android/*.c) \
                      $(wildcard ../lib/sdl2/src/locale/*.c) \
                      $(wildcard ../lib/sdl2/src/misc/*.c) \
                      $(wildcard ../lib/sdl2/src/misc/android/*.c) \
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
                      $(wildcard ../lib/sdl2/src/video/yuv2rgb/*.c)
LOCAL_CFLAGS       := -I../lib/sdl2/include/ -DGL_GLEXT_PROTOTYPES
LOCAL_LDLIBS       := -ldl -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid
LOCAL_STATIC_LIBRARIES := cpufeatures

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

# STK
LOCAL_MODULE       := main
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../src/*.cpp)     \
                      $(wildcard ../src/*/*.cpp)   \
                      $(wildcard ../src/*/*/*.cpp)
LOCAL_LDLIBS       := -llog -lm -lOpenSLES
LOCAL_CFLAGS       := -I../lib/angelscript/include      \
                      -I../lib/bullet/src               \
                      -I../lib/sheenbidi/Headers        \
                      -I../lib/enet/include             \
                      -I../lib/ifaddrs                  \
                      -I../lib/irrlicht/include         \
                      -I../lib/irrlicht/source/Irrlicht \
                      -I../lib/graphics_utils           \
                      -I../lib/graphics_engine/include  \
                      -I../lib/mcpp                     \
                      -I../lib/sdl2/include             \
                      -I../lib/tinygettext/include      \
                      -I../lib/libsquish                \
                      -I../src                          \
                      -Ideps-$(TARGET_ARCH_ABI)/curl/include      \
                      -Ideps-$(TARGET_ARCH_ABI)/freetype/include  \
                      -Ideps-$(TARGET_ARCH_ABI)/harfbuzz/include  \
                      -Ideps-$(TARGET_ARCH_ABI)/libogg/include    \
                      -Ideps-$(TARGET_ARCH_ABI)/libvorbis/include \
                      -Ideps-$(TARGET_ARCH_ABI)/openal/include    \
                      -Ideps-$(TARGET_ARCH_ABI)/mbedtls/include   \
                      -DUSE_GLES2      \
                      -DMOBILE_STK     \
                      -DENABLE_SOUND   \
                      -DENABLE_IPV6    \
                      -DENABLE_CRYPTO_MBEDTLS \
                      -DNDEBUG         \
                      -DDISABLE_ICONV  \
                      -DANDROID_PACKAGE_NAME=\"$(PACKAGE_NAME)\"    \
                      -DANDROID_APP_DIR_NAME=\"$(APP_DIR_NAME)\"    \
                      -DSUPERTUXKART_VERSION=\"$(PROJECT_VERSION)\" \
                      -DANDROID_PACKAGE_CLASS_NAME=\"$(PACKAGE_CLASS_NAME)\"
LOCAL_CPPFLAGS     := -std=gnu++0x

LOCAL_STATIC_LIBRARIES := irrlicht bullet enet ifaddrs angelscript mcpp SDL2 \
                          vorbisfile vorbis ogg openal curl libmbedtls       \
                          libmbedcrypto libmbedx509 c++_static sheenbidi     \
                          harfbuzz freetype tinygettext graphics_utils       \
                          graphics_engine

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_ARM_NEON     := false
endif
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)
$(call import-module, android/cpufeatures)
