# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)


#include $(CLEAR_VARS)
#LOCAL_MODULE    := my_gnustl_shared
#LOCAL_SRC_FILES := external/lib/$(TARGET_ARCH_ABI)/libgnustl_shared.so
#include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)


# PNG

LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE    := png
LOCAL_SRC_FILES = libpng/example.c \
	libpng/png.c \
	libpng/pngerror.c \
	libpng/pngget.c \
	libpng/pngmem.c \
	libpng/pngpread.c \
	libpng/pngread.c \
	libpng/pngrio.c \
	libpng/pngrtran.c \
	libpng/pngrutil.c \
	libpng/pngset.c \
	libpng/pngtest.c \
	libpng/pngtrans.c \
	libpng/pngwio.c \
	libpng/pngwrite.c \
	libpng/pngwtran.c \
	libpng/pngwutil.c \
	zlib/adler32.c \
	zlib/compress.c \
	zlib/crc32.c \
	zlib/deflate.c \
	zlib/gzclose.c \
	zlib/gzlib.c \
	zlib/gzread.c \
	zlib/gzwrite.c \
	zlib/infback.c \
	zlib/inffast.c \
	zlib/inflate.c \
	zlib/inftrees.c \
	zlib/trees.c \
	zlib/uncompr.c \
	zlib/zutil.c
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2
LOCAL_CFLAGS := -I./../include/ -I./include/ -I.
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

# AngelScript
ANGELSCRIPT_INCLUDE := /angelscript/include/

# -----------------------------------------------------
# Build the AngelScript library
# -----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := libangelscript

LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/angelscript/source/*.S)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/angelscript/source/*.cpp)
LOCAL_PATH:=.
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += rtti

LOCAL_PATH := .
# ENET
LOCAL_SRC_FILES := $(wildcard jni/enet/*.c) 
LOCAL_MODULE    := enet
LOCAL_LDLIBS    := -llog -landroid
LOCAL_CFLAGS := -DHAS_SOCKLEN_T -Ijni/ -Ijni/enet/include/
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

# ------------------------------------------------------------------
# Static library for Cocos
# ------------------------------------------------------------------

include $(CLEAR_VARS)


LOCAL_PATH := jni/jpeglib/
LOCAL_MODULE := jpeglib

LOCAL_MODULE_FILENAME := libjpeg

LOCAL_SRC_FILES := \
	jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
	jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
	jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
	jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
	jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c \
	jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
	jfdctint.c jidctflt.c jidctfst.c jidctint.c jquant1.c \
	jquant2.c jutils.c jmemmgr.c jcarith.c jdarith.c jaricom.c

# Use the no backing store memory manager provided by
# libjpeg. See install.txt
LOCAL_SRC_FILES += \
	jmemnobs.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += rtti


# glew
LOCAL_PATH:= jni
LOCAL_SRC_FILES := jni/glew/src/glew.c 
LOCAL_PATH := .
LOCAL_MODULE    := glew
LOCAL_LDLIBS    := -llog -landroid
LOCAL_CFLAGS := -Ijni/glew/include -DGLEW_NO_GLU
#include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += rtti

# Bullet
LOCAL_SRC_FILES := $(wildcard jni/bullet/src/*/*.cpp)
LOCAL_SRC_FILES += $(wildcard jni/bullet/src/*/*/*.cpp)
LOCAL_PATH:=.
LOCAL_MODULE    := bullet
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2
LOCAL_CFLAGS := -Ijni/bullet/src/ -I../include -I../../include
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

# ifaddrs
LOCAL_SRC_FILES := jni/ifaddrs/ifaddrs.c
LOCAL_PATH:=.
LOCAL_MODULE    := ifaddrs
LOCAL_LDLIBS    := -llog -landroid
LOCAL_CFLAGS := -Ijni/ifaddrs/
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)



# Freetype
LOCAL_MODULE := freetype
LOCAL_PATH := .
LOCAL_SRC_FILES := obj/freetype/freetype/lib/libfreetype.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/include/freetype2

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CPP_FEATURES += rtti
LOCAL_PATH:= jni

# Irrlicht
LOCAL_SRC_FILES := $(wildcard jni/irrlicht/source/Irrlicht/*.cpp) $(wildcard jni/irrlicht/source/Irrlicht/Android/*.cpp)
LOCAL_PATH:=.
LOCAL_MODULE    := irrlicht
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2
LOCAL_CFLAGS := -Ijni/irrlicht/source/Irrlicht/ -Ijni/irrlicht/include/ -Ijni/jpeglib/ -Ijni/libpng/ -Ijni/ -Iinclude/ -I$(call my-dir)/../../sources/android/native_app_glue/ -DBUILD_OGLES2 -DNO_IRR_COMPILE_WITH_SOFTWARE_ -DNO_IRR_COMPILE_WITH_BURNINGSVIDEO_

LOCAL_STATIC_LIBRARIES := jpeglib png
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)
LOCAL_PATH:= jni
LOCAL_CPP_FEATURES += rtti exceptions


# STK
LOCAL_SRC_FILES := $(wildcard jni/src/*.cpp) $(wildcard jni/src/*/*.cpp) $(wildcard jni/src/*/*/*.cpp) jni/irrexample.cpp
LOCAL_PATH:=.
LOCAL_MODULE    := stk
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -lGLESv3
LOCAL_CFLAGS := -Ijni/irrlicht/source/Irrlicht/ -Ijni/irrlicht/include/ -Ijni/jpeglib/ -Ijni/libpng/ -Ijni/ -Iinclude/ -I$(call my-dir)/../../sources/android/native_app_glue/ -DBUILD_OGLES2 -DNO_IRR_COMPILE_WITH_SOFTWARE_ -DNO_IRR_COMPILE_WITH_BURNINGSVIDEO_ -DSUPERTUXKART_DATADIR=\"/sdcard/stk/\" -DUSE_GLES2 -DANDROID -Ijni/src/ -Ijni/bullet/src -DNO_CURL -std=c++11 -Iobj/freetype/freetype/include/freetype2/ -Ijni/enet/include/ -Ijni/angelscript/include/ -DDEBUG -DNO_SOUND -DGLEW_NO_GLU -Ijni/ifaddrs

LOCAL_SHARED_LIBRARIES := irrlicht
LOCAL_STATIC_LIBRARIES := bullet enet freetype  ifaddrs angelscript
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)
LOCAL_PATH:= jni


LOCAL_MODULE    := irrlicht2
LOCAL_SRC_FILES := main.c
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2
LOCAL_CFLAGS := -I./jni/irrlicht/include/ -Ijni/irrlicht/source/Irrlicht/ -DNO_CURL -DHAS_SOCKLEN_T -DSUPERTUXKART_DATADIR=\"/sdcard/stk/\"
LOCAL_STATIC_LIBRARIES := android_native_app_glue #jpeg #libirrlicht jpeg png
#LOCAL_SHARED_LIBRARIES := jpeg

include $(BUILD_SHARED_LIBRARY)



$(call import-module,android/native_app_glue)
