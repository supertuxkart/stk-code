# This makefile assumes the ndk-build tool is executed from the sdk/angelscript/projects/android directory
# Change the next line to full path if there are any errors to link or find files
SDK_BASE_PATH := $(call my-dir)/../../../..

ANGELSCRIPT_INCLUDE := $(SDK_BASE_PATH)/angelscript/include/

# -----------------------------------------------------
# Build the AngelScript library
# -----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := libangelscript

# Android API: Checks if can use pthreads. Version 2.3 fully supports threads and atomic instructions
# ifeq ($(TARGET_PLATFORM),android-3)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-4)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-5)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-6)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-7)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-8)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# endif
# endif
# endif
# endif
# endif
# endif

LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES := $(wildcard $(SDK_BASE_PATH)/angelscript/source/*.S)
LOCAL_SRC_FILES += $(wildcard $(SDK_BASE_PATH)/angelscript/source/*.cpp)
LOCAL_PATH := .
LOCAL_ARM_MODE := arm
include $(BUILD_STATIC_LIBRARY)
