// Copyright (C) 2002-2007 Nikolaus Gebhardt
// Copyright (C) 2007-2011 Christian Stehno
// Copyright (C) 2016-2017 Dawid Gan
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifdef ANDROID
#include <irrString.h>
#include <atomic>
#include <jni.h>
#include <SDL_system.h>
#include <vector>
#include "../../../../src/utils/utf8/unchecked.h"

using namespace irr;

// Call when android keyboard is opened or close, and save its height for
// moving screen
std::atomic<int> g_keyboard_height(0);
extern "C" int Android_getKeyboardHeight()
{
    return g_keyboard_height.load();
}

#define MAKE_ANDROID_SAVE_KBD_HEIGHT_CALLBACK(x) JNIEXPORT void JNICALL Java_ ## x##_SuperTuxKartActivity_saveKeyboardHeight(JNIEnv* env, jobject this_obj, jint height)
#define ANDROID_SAVE_KBD_HEIGHT_CALLBACK(PKG_NAME) MAKE_ANDROID_SAVE_KBD_HEIGHT_CALLBACK(PKG_NAME)

extern "C"
ANDROID_SAVE_KBD_HEIGHT_CALLBACK(ANDROID_PACKAGE_CALLBACK_NAME)
{
    g_keyboard_height.store((int)height);
}

bool Android_isHardwareKeyboardConnected()
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env)
    {
        return false;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity)
    {
        return false;
    }

    jclass class_native_activity = env->GetObjectClass(activity);
    if (class_native_activity == NULL)
    {
        env->DeleteLocalRef(activity);
        return false;
    }

    jmethodID method_id = env->GetMethodID(class_native_activity, "isHardwareKeyboardConnected", "()Z");
    if (method_id == NULL)
    {
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(activity);
        return false;
    }

    bool ret = env->CallBooleanMethod(activity, method_id);
    env->DeleteLocalRef(class_native_activity);
    env->DeleteLocalRef(activity);
    return ret;
}

void Android_toggleOnScreenKeyboard(bool show, int type, int y)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env)
    {
        return;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity)
    {
        return;
    }

    jclass class_native_activity = env->GetObjectClass(activity);
    if (class_native_activity == NULL)
    {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID method_id = NULL;
    if (show)
        method_id = env->GetMethodID(class_native_activity, "showKeyboard", "(II)V");
    else
        method_id = env->GetMethodID(class_native_activity, "hideKeyboard", "(Z)V");

    if (method_id == NULL)
    {
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(activity);
        return;
    }

    if (show)
        env->CallVoidMethod(activity, method_id, (jint)type, (jint)y);
    else
        env->CallVoidMethod(activity, method_id, (jboolean)(type != 0));
    env->DeleteLocalRef(class_native_activity);
    env->DeleteLocalRef(activity);
}

void Android_fromSTKEditBox(int widget_id, const core::stringw& text, int selection_start, int selection_end, int type)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env)
    {
        return;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity)
    {
        return;
    }

    jclass class_native_activity = env->GetObjectClass(activity);
    if (class_native_activity == NULL)
    {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID method_id = env->GetMethodID(class_native_activity, "fromSTKEditBox", "(ILjava/lang/String;III)V");
    if (method_id == NULL)
    {
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(activity);
        return;
    }

    // Android use 32bit wchar_t and java use utf16 string
    // We should not use the modified utf8 from java as it fails for emoji
    // because it's larger than 16bit

    std::vector<uint16_t> utf16;
    // Use utf32 for emoji later
    static_assert(sizeof(wchar_t) == sizeof(uint32_t), "wchar_t is not 32bit");
    const uint32_t* chars = (const uint32_t*)text.c_str();
    utf8::unchecked::utf32to16(chars, chars + text.size(), back_inserter(utf16));

    std::vector<int> mappings;
    int pos = 0;
    mappings.push_back(pos++);
    for (unsigned i = 0; i < utf16.size(); i++)
    {
        if (utf8::internal::is_lead_surrogate(utf16[i]))
        {
            pos++;
            mappings.push_back(pos++);
            i++;
        }
        else
            mappings.push_back(pos++);
    }

    // Correct start / end position for utf16
    if (selection_start < (int)mappings.size())
        selection_start = mappings[selection_start];
    if (selection_end < (int)mappings.size())
        selection_end = mappings[selection_end];

    jstring jstring_text = env->NewString((const jchar*)utf16.data(), utf16.size());

    env->CallVoidMethod(activity, method_id, (jint)widget_id, jstring_text, (jint)selection_start, (jint)selection_end, (jint)type);
    env->DeleteLocalRef(jstring_text);
    env->DeleteLocalRef(class_native_activity);
    env->DeleteLocalRef(activity);
}

#endif
