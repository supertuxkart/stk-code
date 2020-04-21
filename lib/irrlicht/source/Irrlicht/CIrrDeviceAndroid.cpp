// Copyright (C) 2002-2007 Nikolaus Gebhardt
// Copyright (C) 2007-2011 Christian Stehno
// Copyright (C) 2016-2017 Dawid Gan
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CIrrDeviceAndroid.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include <assert.h>
#include <atomic>
#include <mutex>
#include <vector>
#include "os.h"
#include "CContextEGL.h"
#include "CFileSystem.h"
#include "COGLES2Driver.h"
#include "MobileCursorControl.h"
#include "../../../../src/utils/utf8/unchecked.h"

#include <SDL.h>

// Call when android keyboard is opened or close, and save its height for
// moving screen
std::atomic<int> g_keyboard_height(0);
std::mutex g_event_mutex;
std::vector<irr::SEvent> g_events;
std::map<int, irr::EKEY_CODE> g_keymap;

#define MAKE_ANDROID_SAVE_KBD_HEIGHT_CALLBACK(x) JNIEXPORT void JNICALL Java_ ## x##_SuperTuxKartActivity_saveKeyboardHeight(JNIEnv* env, jobject this_obj, jint height)
#define ANDROID_SAVE_KBD_HEIGHT_CALLBACK(PKG_NAME) MAKE_ANDROID_SAVE_KBD_HEIGHT_CALLBACK(PKG_NAME)

extern "C"
ANDROID_SAVE_KBD_HEIGHT_CALLBACK(ANDROID_PACKAGE_CALLBACK_NAME)
{
    g_keyboard_height.store((int)height);
}

#define ADD_TOUCH_CALLBACK(x) JNIEXPORT void JNICALL Java_ ## x##_SuperTuxKartActivity_addTouch(JNIEnv* env, jobject this_obj, jint id, jint X, jint Y, jint event_type)
#define ANDROID_ADD_TOUCH_CALLBACK(PKG_NAME) ADD_TOUCH_CALLBACK(PKG_NAME)
extern "C"
ANDROID_ADD_TOUCH_CALLBACK(ANDROID_PACKAGE_CALLBACK_NAME)
{
    irr::SEvent event;
    event.EventType = irr::EET_TOUCH_INPUT_EVENT;
    event.TouchInput.ID = id;
    event.TouchInput.X = X;
    event.TouchInput.Y = Y;
    event.TouchInput.Event = (irr::ETOUCH_INPUT_EVENT)event_type;
    std::lock_guard<std::mutex> lock(g_event_mutex);
    g_events.push_back(event);
}

extern "C" void stkAddKeyEvent(int key_code, int action, int meta_state, int scan_code, int unichar)
{
    irr::SEvent event;
    event.EventType = irr::EET_KEY_INPUT_EVENT;
    event.KeyInput.Char = unichar;
    event.KeyInput.PressedDown = false;
    event.KeyInput.Key = irr::IRR_KEY_UNKNOWN;

    if (action == AKEY_EVENT_ACTION_DOWN)
    {
        event.KeyInput.PressedDown = true;
    }
    else if (action == AKEY_EVENT_ACTION_UP)
    {
        event.KeyInput.PressedDown = false;
    }

    event.KeyInput.Shift = (meta_state & AMETA_SHIFT_ON ||
                            meta_state & AMETA_SHIFT_LEFT_ON ||
                            meta_state & AMETA_SHIFT_RIGHT_ON);

    event.KeyInput.Control = (meta_state & AMETA_CTRL_ON ||
                              meta_state & AMETA_CTRL_LEFT_ON ||
                              meta_state & AMETA_CTRL_RIGHT_ON);

    event.KeyInput.SystemKeyCode = key_code;
    event.KeyInput.Key = g_keymap[key_code];

    // If button doesn't return key code, then at least use device-specific
    // scan code, because it's better than nothing
    if (event.KeyInput.Key == 0)
        event.KeyInput.Key = (irr::EKEY_CODE)((int)irr::IRR_KEY_CODES_COUNT + scan_code);

    std::lock_guard<std::mutex> lock(g_event_mutex);
    g_events.push_back(event);
}

#define ADD_KEY_CALLBACK(x) JNIEXPORT void JNICALL Java_ ## x##_SuperTuxKartActivity_addKey(JNIEnv* env, jobject this_obj, jint key_code, jint action, jint meta_state, jint scan_code, jint unichar)
#define ANDROID_ADD_KEY_CALLBACK(PKG_NAME) ADD_KEY_CALLBACK(PKG_NAME)

extern "C"
ANDROID_ADD_KEY_CALLBACK(ANDROID_PACKAGE_CALLBACK_NAME)
{
    stkAddKeyEvent(key_code, action, meta_state, scan_code, unichar);
}

namespace irr
{
    namespace video
    {
        IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
            video::SExposedVideoData& data, io::IFileSystem* io, IrrlichtDevice* device);
    }
}

namespace irr
{

// These variables must be global. Otherwise initialization will reach infinite
// loop after creating the device second time (i.e. the NULL driver and then
// GLES2 driver). We get initialization events from Android only once.
bool CIrrDeviceAndroid::IsPaused = true;
bool CIrrDeviceAndroid::IsFocused = false;
bool CIrrDeviceAndroid::IsStarted = false;

AndroidApplicationInfo CIrrDeviceAndroid::ApplicationInfo;

// Execution of android_main() function is a kind of "onCreate" event, so this
// function should be used there to make sure that global window state variables
// have their default values on startup.
void CIrrDeviceAndroid::onCreate()
{
    IsPaused = true;
    IsFocused = false;
    IsStarted = false;
}

JNIEnv* CIrrDeviceAndroid::m_jni_env = NULL;

//! constructor
CIrrDeviceAndroid::CIrrDeviceAndroid(const SIrrlichtCreationParameters& param)
    : CIrrDeviceStub(param),
    Accelerometer(0),
    Gyroscope(0),
    AccelerometerActive(false),
    GyroscopeActive(false),
    HasTouchDevice(false),
    GamepadAxisX(0),
    GamepadAxisY(0),
    DefaultOrientation(ORIENTATION_UNKNOWN)
{
    #ifdef _DEBUG
    setDebugName("CIrrDeviceAndroid");
    #endif
    m_screen_height = 0;
    m_moved_height = 0;
    m_moved_height_func = NULL;
    m_jni_env = NULL;

    CursorControl = new gui::MobileCursorControl();

    Android = (android_app*)(param.PrivateData);
    
    if (Android == NULL && CreationParams.DriverType != video::EDT_NULL)
    {
        os::Printer::log("Irrlicht device can run only with NULL driver without android_app.", ELL_DEBUG);
        return;
    }

    if (Android != NULL)
    {
        m_jni_env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        Android->userData = this;
        Android->onAppCmd = handleAndroidCommand;
        Android->onAppCmdDirect = NULL;
        Android->onInputEvent = NULL;
        
        printConfig();
        
        Close = Android->destroyRequested;
    
        // It typically shouldn't happen, but just in case...
        if (Close)
            return;
    
        SensorManager = ASensorManager_getInstance();
        SensorEventQueue = ASensorManager_createEventQueue(SensorManager,
                                    Android->looper, LOOPER_ID_USER, NULL, NULL);
    
        ANativeActivity_setWindowFlags(Android->activity,
                                       AWINDOW_FLAG_KEEP_SCREEN_ON |
                                       AWINDOW_FLAG_FULLSCREEN, 0);
    
        os::Printer::log("Waiting for Android activity window to be created.", ELL_DEBUG);
    
        while (!IsStarted || !IsFocused || IsPaused)
        {
            s32 events = 0;
            android_poll_source* source = 0;
    
            s32 id = ALooper_pollAll(-1, NULL, &events, (void**)&source);
    
            if (id >=0 && source != NULL)
            {
                source->process(Android, source);
            }
        }
        
        assert(Android->window);
        os::Printer::log("Done", ELL_DEBUG);
        
        ExposedVideoData.OGLESAndroid.Window = Android->window;
    
        createVideoModeList();
        
        int32_t touch = AConfiguration_getTouchscreen(Android->config);
        HasTouchDevice = touch != ACONFIGURATION_TOUCHSCREEN_NOTOUCH;
    }

    createDriver();

    if (VideoDriver)
    {
        createGUIAndScene();
    }
}


CIrrDeviceAndroid::~CIrrDeviceAndroid()
{
    if (Android)
    {
        Android->userData = NULL;
        Android->onAppCmd = NULL;
        Android->onInputEvent = NULL;
    }
}

void CIrrDeviceAndroid::printConfig() 
{
    char language[3] = {};
    char country[3] = {};
    AConfiguration_getLanguage(Android->config, language);
    AConfiguration_getCountry(Android->config, country);
    int32_t mcc = AConfiguration_getMcc(Android->config);
    int32_t mnc = AConfiguration_getMnc(Android->config);
    int32_t orientation = AConfiguration_getOrientation(Android->config);
    int32_t touch = AConfiguration_getTouchscreen(Android->config);
    int32_t density = AConfiguration_getDensity(Android->config);
    int32_t keyboard = AConfiguration_getKeyboard(Android->config);
    int32_t navigation = AConfiguration_getNavigation(Android->config);
    int32_t keys_hidden = AConfiguration_getKeysHidden(Android->config);
    int32_t nav_hidden = AConfiguration_getNavHidden(Android->config);
    int32_t sdk_version = AConfiguration_getSdkVersion(Android->config);
    int32_t screen_size = AConfiguration_getScreenSize(Android->config);
    int32_t screen_long = AConfiguration_getScreenLong(Android->config);
    int32_t ui_mode_type = AConfiguration_getUiModeType(Android->config);
    int32_t ui_mode_night = AConfiguration_getUiModeNight(Android->config);

    os::Printer::log("Android configuration: ", ELL_DEBUG);
    os::Printer::log("   country:", !std::string(country).empty() ? country : "unknown", ELL_DEBUG);
    os::Printer::log("   density:", core::stringc(density).c_str(), ELL_DEBUG);
    os::Printer::log("   keyboard:", core::stringc(keyboard).c_str(), ELL_DEBUG);
    os::Printer::log("   keys_hidden:", core::stringc(keys_hidden).c_str(), ELL_DEBUG);
    os::Printer::log("   language:", !std::string(language).empty() ? language : "unknown", ELL_DEBUG);
    os::Printer::log("   mcc:", core::stringc(mcc).c_str(), ELL_DEBUG);
    os::Printer::log("   mnc:", core::stringc(mnc).c_str(), ELL_DEBUG);
    os::Printer::log("   nav_hidden:", core::stringc(nav_hidden).c_str(), ELL_DEBUG);
    os::Printer::log("   navigation:", core::stringc(navigation).c_str(), ELL_DEBUG);
    os::Printer::log("   orientation:", core::stringc(orientation).c_str(), ELL_DEBUG);
    os::Printer::log("   screen_long:", core::stringc(screen_long).c_str(), ELL_DEBUG);
    os::Printer::log("   screen_size:", core::stringc(screen_size).c_str(), ELL_DEBUG);
    os::Printer::log("   sdk_version:", core::stringc(sdk_version).c_str(), ELL_DEBUG);
    os::Printer::log("   touch:", core::stringc(touch).c_str(), ELL_DEBUG);
    os::Printer::log("   ui_mode_type:", core::stringc(ui_mode_type).c_str(), ELL_DEBUG);
    os::Printer::log("   ui_mode_night:", core::stringc(ui_mode_night).c_str(), ELL_DEBUG);
}

u32 CIrrDeviceAndroid::getOnScreenKeyboardHeight() const
{
    int height = g_keyboard_height.load();
    if (height > 0)
        return height;
    return 0;
}

void CIrrDeviceAndroid::createVideoModeList()
{
    if (VideoModeList.getVideoModeCount() > 0)
        return;
        
    int width = ANativeWindow_getWidth(Android->window);
    int height = ANativeWindow_getHeight(Android->window);

    os::Printer::log("Window width:", core::stringc(width).c_str(), ELL_DEBUG);
    os::Printer::log("Window height:", core::stringc(height).c_str(), ELL_DEBUG);
    
    if (width > 0 && height > 0)
    {
        CreationParams.WindowSize.Width = width;
        CreationParams.WindowSize.Height = height;
        m_screen_height = height;
    }

    core::dimension2d<u32> size = core::dimension2d<u32>(
                                    CreationParams.WindowSize.Width,
                                    CreationParams.WindowSize.Height);

    VideoModeList.addMode(size, 32);
    VideoModeList.setDesktop(32, size);
}

void CIrrDeviceAndroid::createDriver()
{
    // Create the driver.
    switch(CreationParams.DriverType)
    {
    case video::EDT_OGLES2:
        #ifdef _IRR_COMPILE_WITH_OGLES2_
        VideoDriver = video::createOGLES2Driver(CreationParams, ExposedVideoData, FileSystem, this);
        #else
        os::Printer::log("No OpenGL ES 2.0 support compiled in.", ELL_ERROR);
        #endif
        break;

    case video::EDT_NULL:
        VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
        break;

    default:
        os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
        break;
    }
}

bool CIrrDeviceAndroid::run()
{
    os::Timer::tick();
    
    if (Android == NULL)
        return !Close;
    
    while (!Close)
    {
        if (m_moved_height_func != NULL)
            m_moved_height = m_moved_height_func(this);

        // Input events
        std::vector<SEvent> events;
        std::unique_lock<std::mutex> ul(g_event_mutex);
        std::swap(events, g_events);
        ul.unlock();
        for (auto& e : events)
        {
            bool simulate_mouse = false;
            core::position2d<s32> mouse_pos;
            if (e.EventType == EET_TOUCH_INPUT_EVENT)
            {
                if (e.TouchInput.ID == 0)
                {
                    simulate_mouse = true;
                    mouse_pos.X = e.TouchInput.X;
                    mouse_pos.Y = e.TouchInput.Y;
                }
            }
            postEventFromUser(e);
            // Simulate mouse event for first finger on multitouch device.
            // This allows to click on GUI elements.
            if (simulate_mouse)
                simulateMouse(e, mouse_pos);
        }

        s32 Events = 0;
        android_poll_source* Source = 0;
        bool should_run = (IsStarted && IsFocused && !IsPaused);
        s32 id = ALooper_pollAll(should_run ? 0 : -1, NULL, &Events,
                                 (void**)&Source);

        if (id < 0)
            break;

        if (Source)
        {
            Source->process(Android, Source);
        }

        // if a sensor has data, we'll process it now.
        if (id == LOOPER_ID_USER)
        {
            ASensorEvent event;
            while (ASensorEventQueue_getEvents(SensorEventQueue, &event, 1) > 0)
            {
                switch (event.type)
                {
                case ASENSOR_TYPE_ACCELEROMETER:
                    SEvent accEvent;
                    accEvent.EventType = EET_ACCELEROMETER_EVENT;
                    
                    if (DefaultOrientation == ORIENTATION_LANDSCAPE)
                    {
                        accEvent.AccelerometerEvent.X = event.acceleration.y;
                        accEvent.AccelerometerEvent.Y = -event.acceleration.x;
                    }
                    else
                    {
                        accEvent.AccelerometerEvent.X = event.acceleration.x;
                        accEvent.AccelerometerEvent.Y = event.acceleration.y;
                    }
                    
                    if (accEvent.AccelerometerEvent.X < 0)
                    {
                        accEvent.AccelerometerEvent.X *= -1;
                        accEvent.AccelerometerEvent.Y *= -1;
                    }
                    
                    accEvent.AccelerometerEvent.Z = event.acceleration.z;

                    postEventFromUser(accEvent);
                    break;

                case ASENSOR_TYPE_GYROSCOPE:
                    SEvent gyroEvent;
                    gyroEvent.EventType = EET_GYROSCOPE_EVENT;
                    gyroEvent.GyroscopeEvent.X = event.vector.x;
                    gyroEvent.GyroscopeEvent.Y = event.vector.y;
                    gyroEvent.GyroscopeEvent.Z = event.vector.z;

                    postEventFromUser(gyroEvent);
                    break;
                default:
                    break;
                }
            }
        }
    }

    return !Close;
}

void CIrrDeviceAndroid::yield()
{
    struct timespec ts = {0,1};
    nanosleep(&ts, NULL);
}

void CIrrDeviceAndroid::sleep(u32 timeMs, bool pauseTimer)
{
    const bool wasStopped = Timer ? Timer->isStopped() : true;

    struct timespec ts;
    ts.tv_sec = (time_t) (timeMs / 1000);
    ts.tv_nsec = (long) (timeMs % 1000) * 1000000;

    if (pauseTimer && !wasStopped)
        Timer->stop();

    nanosleep(&ts, NULL);

    if (pauseTimer && !wasStopped)
        Timer->start();
}

void CIrrDeviceAndroid::setWindowCaption(const wchar_t* text)
{
}

bool CIrrDeviceAndroid::present(video::IImage* surface, void* windowId,
                                core::rect<s32>* srcClip)
{
    return true;
}

bool CIrrDeviceAndroid::isWindowActive() const
{
    return (IsFocused && !IsPaused);
}

bool CIrrDeviceAndroid::isWindowFocused() const
{
    return IsFocused;
}

bool CIrrDeviceAndroid::isWindowMinimized() const
{
    return IsPaused;
}

void CIrrDeviceAndroid::closeDevice()
{
    Close = true;
}

void CIrrDeviceAndroid::setResizable(bool resize)
{
}

void CIrrDeviceAndroid::minimizeWindow()
{
}

void CIrrDeviceAndroid::maximizeWindow()
{
}

void CIrrDeviceAndroid::restoreWindow()
{
}

bool CIrrDeviceAndroid::moveWindow(int x, int y)
{
    return true;
}

bool CIrrDeviceAndroid::getWindowPosition(int* x, int* y)
{
    *x = 0;
    *y = 0;
    return true;
}

E_DEVICE_TYPE CIrrDeviceAndroid::getType() const
{
    return EIDT_ANDROID;
}

void CIrrDeviceAndroid::handleAndroidCommand(android_app* app, int32_t cmd)
{
    CIrrDeviceAndroid* device = (CIrrDeviceAndroid *)app->userData;
    assert(device != NULL);
    
    switch (cmd)
    {
    case APP_CMD_SAVE_STATE:
        os::Printer::log("Android command APP_CMD_SAVE_STATE", ELL_DEBUG);
        break;
    case APP_CMD_INIT_WINDOW:
        os::Printer::log("Android command APP_CMD_INIT_WINDOW", ELL_DEBUG);
        
        device->getExposedVideoData().OGLESAndroid.Window = app->window;
        
        // If the Android app is resumed, we need to re-create EGL surface
        // to allow to draw something on it again.
        if (device->VideoDriver != NULL && 
            device->CreationParams.DriverType == video::EDT_OGLES2)
        {
            video::COGLES2Driver* driver = (video::COGLES2Driver*)(device->VideoDriver);
            driver->getEGLContext()->reloadEGLSurface(app->window);
        }
        
        IsStarted = true;
        break;
    case APP_CMD_TERM_WINDOW:
        os::Printer::log("Android command APP_CMD_TERM_WINDOW", ELL_DEBUG);
        IsStarted = false;
        break;
    case APP_CMD_GAINED_FOCUS:
        os::Printer::log("Android command APP_CMD_GAINED_FOCUS", ELL_DEBUG);
        IsFocused = true;
        break;
    case APP_CMD_LOST_FOCUS:
        os::Printer::log("Android command APP_CMD_LOST_FOCUS", ELL_DEBUG);
        IsFocused = false;
        break;
    case APP_CMD_DESTROY:
        os::Printer::log("Android command APP_CMD_DESTROY", ELL_DEBUG);
        device->Close = true;
        // Make sure that state variables are set to the default state
        // when the app is destroyed
        IsPaused = true;
        IsFocused = false;
        IsStarted = false;
        break;
    case APP_CMD_PAUSE:
        os::Printer::log("Android command APP_CMD_PAUSE", ELL_DEBUG);
        IsPaused = true;
        break;
    case APP_CMD_RESUME:
        os::Printer::log("Android command APP_CMD_RESUME", ELL_DEBUG);
        IsPaused = false;
        break;
    case APP_CMD_START:
        os::Printer::log("Android command APP_CMD_START", ELL_DEBUG);
        break;
    case APP_CMD_STOP:
        os::Printer::log("Android command APP_CMD_STOP", ELL_DEBUG);
        break;
    case APP_CMD_WINDOW_RESIZED:
        os::Printer::log("Android command APP_CMD_WINDOW_RESIZED", ELL_DEBUG);
        break;
    case APP_CMD_CONFIG_CHANGED:
        os::Printer::log("Android command APP_CMD_CONFIG_CHANGED", ELL_DEBUG);
        break;
    case APP_CMD_LOW_MEMORY:
        os::Printer::log("Android command APP_CMD_LOW_MEMORY", ELL_DEBUG);
        break;
    default:
        os::Printer::log("Android command: ", core::stringc(cmd).c_str(), ELL_DEBUG);
        break;
    }
    
    SEvent event;
    event.EventType = EET_SYSTEM_EVENT;
    event.SystemEvent.EventType = ESET_ANDROID_CMD;
    event.SystemEvent.AndroidCmd.Cmd = cmd;
    device->postEventFromUser(event);
}

video::SExposedVideoData& CIrrDeviceAndroid::getExposedVideoData()
{
    return ExposedVideoData;
}

// Create the keymap in startSTK
extern "C" void createKeyMap()
{
    g_keymap[AKEYCODE_UNKNOWN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_SOFT_LEFT] = IRR_KEY_LBUTTON;
    g_keymap[AKEYCODE_SOFT_RIGHT] = IRR_KEY_RBUTTON;
    g_keymap[AKEYCODE_HOME] = IRR_KEY_HOME;
    g_keymap[AKEYCODE_BACK] = IRR_KEY_ESCAPE;
    g_keymap[AKEYCODE_CALL] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_ENDCALL] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_0] = IRR_KEY_0;
    g_keymap[AKEYCODE_1] = IRR_KEY_1;
    g_keymap[AKEYCODE_2] = IRR_KEY_2;
    g_keymap[AKEYCODE_3] = IRR_KEY_3;
    g_keymap[AKEYCODE_4] = IRR_KEY_4;
    g_keymap[AKEYCODE_5] = IRR_KEY_5;
    g_keymap[AKEYCODE_6] = IRR_KEY_6;
    g_keymap[AKEYCODE_7] = IRR_KEY_7;
    g_keymap[AKEYCODE_8] = IRR_KEY_8;
    g_keymap[AKEYCODE_9] = IRR_KEY_9;
    g_keymap[AKEYCODE_STAR] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_POUND] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_DPAD_UP] = IRR_KEY_UP;
    g_keymap[AKEYCODE_DPAD_DOWN] = IRR_KEY_DOWN;
    g_keymap[AKEYCODE_DPAD_LEFT] = IRR_KEY_LEFT;
    g_keymap[AKEYCODE_DPAD_RIGHT] = IRR_KEY_RIGHT;
    g_keymap[AKEYCODE_DPAD_CENTER] = IRR_KEY_RETURN;
    g_keymap[AKEYCODE_VOLUME_UP] = IRR_KEY_VOLUME_DOWN;
    g_keymap[AKEYCODE_VOLUME_DOWN] = IRR_KEY_VOLUME_UP;
    g_keymap[AKEYCODE_POWER] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CAMERA] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CLEAR] = IRR_KEY_CLEAR;
    g_keymap[AKEYCODE_A] = IRR_KEY_A;
    g_keymap[AKEYCODE_B] = IRR_KEY_B;
    g_keymap[AKEYCODE_C] = IRR_KEY_C;
    g_keymap[AKEYCODE_D] = IRR_KEY_D;
    g_keymap[AKEYCODE_E] = IRR_KEY_E;
    g_keymap[AKEYCODE_F] = IRR_KEY_F;
    g_keymap[AKEYCODE_G] = IRR_KEY_G;
    g_keymap[AKEYCODE_H] = IRR_KEY_H;
    g_keymap[AKEYCODE_I] = IRR_KEY_I;
    g_keymap[AKEYCODE_J] = IRR_KEY_J;
    g_keymap[AKEYCODE_K] = IRR_KEY_K;
    g_keymap[AKEYCODE_L] = IRR_KEY_L;
    g_keymap[AKEYCODE_M] = IRR_KEY_M;
    g_keymap[AKEYCODE_N] = IRR_KEY_N;
    g_keymap[AKEYCODE_O] = IRR_KEY_O;
    g_keymap[AKEYCODE_P] = IRR_KEY_P;
    g_keymap[AKEYCODE_Q] = IRR_KEY_Q;
    g_keymap[AKEYCODE_R] = IRR_KEY_R;
    g_keymap[AKEYCODE_S] = IRR_KEY_S;
    g_keymap[AKEYCODE_T] = IRR_KEY_T;
    g_keymap[AKEYCODE_U] = IRR_KEY_U;
    g_keymap[AKEYCODE_V] = IRR_KEY_V;
    g_keymap[AKEYCODE_W] = IRR_KEY_W;
    g_keymap[AKEYCODE_X] = IRR_KEY_X;
    g_keymap[AKEYCODE_Y] = IRR_KEY_Y;
    g_keymap[AKEYCODE_Z] = IRR_KEY_Z;
    g_keymap[AKEYCODE_COMMA] = IRR_KEY_COMMA;
    g_keymap[AKEYCODE_PERIOD] = IRR_KEY_PERIOD;
    g_keymap[AKEYCODE_ALT_LEFT] = IRR_KEY_MENU;
    g_keymap[AKEYCODE_ALT_RIGHT] = IRR_KEY_MENU;
    g_keymap[AKEYCODE_SHIFT_LEFT] = IRR_KEY_LSHIFT;
    g_keymap[AKEYCODE_SHIFT_RIGHT] = IRR_KEY_RSHIFT;
    g_keymap[AKEYCODE_TAB] = IRR_KEY_TAB;
    g_keymap[AKEYCODE_SPACE] = IRR_KEY_SPACE;
    g_keymap[AKEYCODE_SYM] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_EXPLORER] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_ENVELOPE] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_ENTER] = IRR_KEY_RETURN;
    g_keymap[AKEYCODE_DEL] = IRR_KEY_BACK;
    g_keymap[AKEYCODE_GRAVE] = IRR_KEY_OEM_3;
    g_keymap[AKEYCODE_MINUS] = IRR_KEY_MINUS;
    g_keymap[AKEYCODE_EQUALS] = IRR_KEY_PLUS;
    g_keymap[AKEYCODE_LEFT_BRACKET] = IRR_KEY_OEM_4;
    g_keymap[AKEYCODE_RIGHT_BRACKET] = IRR_KEY_OEM_6;
    g_keymap[AKEYCODE_BACKSLASH] = IRR_KEY_OEM_5;
    g_keymap[AKEYCODE_SEMICOLON] = IRR_KEY_OEM_1;
    g_keymap[AKEYCODE_APOSTROPHE] = IRR_KEY_OEM_7;
    g_keymap[AKEYCODE_SLASH] = IRR_KEY_OEM_2;
    g_keymap[AKEYCODE_AT] = IRR_KEY_2;
    g_keymap[AKEYCODE_NUM] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_HEADSETHOOK] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_FOCUS] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_PLUS] = IRR_KEY_PLUS;
    g_keymap[AKEYCODE_MENU] = IRR_KEY_MENU;
    g_keymap[AKEYCODE_NOTIFICATION] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_SEARCH] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MEDIA_PLAY_PAUSE] = IRR_KEY_MEDIA_PLAY_PAUSE;
    g_keymap[AKEYCODE_MEDIA_STOP] = IRR_KEY_MEDIA_STOP;
    g_keymap[AKEYCODE_MEDIA_NEXT] = IRR_KEY_MEDIA_NEXT_TRACK;
    g_keymap[AKEYCODE_MEDIA_PREVIOUS] = IRR_KEY_MEDIA_PREV_TRACK;
    g_keymap[AKEYCODE_MEDIA_REWIND] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MEDIA_FAST_FORWARD] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MUTE] = IRR_KEY_VOLUME_MUTE;
    g_keymap[AKEYCODE_PAGE_UP] = IRR_KEY_PRIOR;
    g_keymap[AKEYCODE_PAGE_DOWN] = IRR_KEY_NEXT;
    g_keymap[AKEYCODE_PICTSYMBOLS] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_SWITCH_CHARSET] = IRR_KEY_UNKNOWN;

    // following look like controller inputs
    g_keymap[AKEYCODE_BUTTON_A] = IRR_KEY_BUTTON_A;
    g_keymap[AKEYCODE_BUTTON_B] = IRR_KEY_BUTTON_B;
    g_keymap[AKEYCODE_BUTTON_C] = IRR_KEY_BUTTON_C;
    g_keymap[AKEYCODE_BUTTON_X] = IRR_KEY_BUTTON_X;
    g_keymap[AKEYCODE_BUTTON_Y] = IRR_KEY_BUTTON_Y;
    g_keymap[AKEYCODE_BUTTON_Z] = IRR_KEY_BUTTON_Z;
    g_keymap[AKEYCODE_BUTTON_L1] = IRR_KEY_BUTTON_L1;
    g_keymap[AKEYCODE_BUTTON_R1] = IRR_KEY_BUTTON_R1;
    g_keymap[AKEYCODE_BUTTON_L2] = IRR_KEY_BUTTON_L2;
    g_keymap[AKEYCODE_BUTTON_R2] = IRR_KEY_BUTTON_R2;
    g_keymap[AKEYCODE_BUTTON_THUMBL] = IRR_KEY_BUTTON_THUMBL;
    g_keymap[AKEYCODE_BUTTON_THUMBR] = IRR_KEY_BUTTON_THUMBR;
    g_keymap[AKEYCODE_BUTTON_START] = IRR_KEY_BUTTON_START;
    g_keymap[AKEYCODE_BUTTON_SELECT] = IRR_KEY_BUTTON_SELECT;
    g_keymap[AKEYCODE_BUTTON_MODE] = IRR_KEY_BUTTON_MODE;

    g_keymap[AKEYCODE_ESCAPE] = IRR_KEY_ESCAPE;
    g_keymap[AKEYCODE_FORWARD_DEL] = IRR_KEY_DELETE;
    g_keymap[AKEYCODE_CTRL_LEFT] = IRR_KEY_CONTROL;
    g_keymap[AKEYCODE_CTRL_RIGHT] = IRR_KEY_CONTROL;
    g_keymap[AKEYCODE_CAPS_LOCK] = IRR_KEY_CAPITAL;
    g_keymap[AKEYCODE_SCROLL_LOCK] = IRR_KEY_SCROLL;
    g_keymap[AKEYCODE_META_LEFT] = IRR_KEY_LWIN;
    g_keymap[AKEYCODE_META_RIGHT] = IRR_KEY_RWIN;
    g_keymap[AKEYCODE_FUNCTION] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_SYSRQ] = IRR_KEY_SNAPSHOT;
    g_keymap[AKEYCODE_BREAK] = IRR_KEY_PAUSE;
    g_keymap[AKEYCODE_MOVE_HOME] = IRR_KEY_HOME;
    g_keymap[AKEYCODE_MOVE_END] = IRR_KEY_END;
    g_keymap[AKEYCODE_INSERT] = IRR_KEY_INSERT;
    g_keymap[AKEYCODE_FORWARD] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MEDIA_PLAY] = IRR_KEY_PLAY;
    g_keymap[AKEYCODE_MEDIA_PAUSE] = IRR_KEY_MEDIA_PLAY_PAUSE;
    g_keymap[AKEYCODE_MEDIA_CLOSE] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MEDIA_EJECT] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MEDIA_RECORD] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_F1] = IRR_KEY_F1;
    g_keymap[AKEYCODE_F2] = IRR_KEY_F2;
    g_keymap[AKEYCODE_F3] = IRR_KEY_F3;
    g_keymap[AKEYCODE_F4] = IRR_KEY_F4;
    g_keymap[AKEYCODE_F5] = IRR_KEY_F5;
    g_keymap[AKEYCODE_F6] = IRR_KEY_F6;
    g_keymap[AKEYCODE_F7] = IRR_KEY_F7;
    g_keymap[AKEYCODE_F8] = IRR_KEY_F8;
    g_keymap[AKEYCODE_F9] = IRR_KEY_F9;
    g_keymap[AKEYCODE_F10] = IRR_KEY_F10;
    g_keymap[AKEYCODE_F11] = IRR_KEY_F11;
    g_keymap[AKEYCODE_F12] = IRR_KEY_F12;
    g_keymap[AKEYCODE_NUM_LOCK] = IRR_KEY_NUMLOCK;
    g_keymap[AKEYCODE_NUMPAD_0] = IRR_KEY_NUMPAD0;
    g_keymap[AKEYCODE_NUMPAD_1] = IRR_KEY_NUMPAD1;
    g_keymap[AKEYCODE_NUMPAD_2] = IRR_KEY_NUMPAD2;
    g_keymap[AKEYCODE_NUMPAD_3] = IRR_KEY_NUMPAD3;
    g_keymap[AKEYCODE_NUMPAD_4] = IRR_KEY_NUMPAD4;
    g_keymap[AKEYCODE_NUMPAD_5] = IRR_KEY_NUMPAD5;
    g_keymap[AKEYCODE_NUMPAD_6] = IRR_KEY_NUMPAD6;
    g_keymap[AKEYCODE_NUMPAD_7] = IRR_KEY_NUMPAD7;
    g_keymap[AKEYCODE_NUMPAD_8] = IRR_KEY_NUMPAD8;
    g_keymap[AKEYCODE_NUMPAD_9] = IRR_KEY_NUMPAD9;
    g_keymap[AKEYCODE_NUMPAD_DIVIDE] = IRR_KEY_DIVIDE;
    g_keymap[AKEYCODE_NUMPAD_MULTIPLY] = IRR_KEY_MULTIPLY;
    g_keymap[AKEYCODE_NUMPAD_SUBTRACT] = IRR_KEY_SUBTRACT;
    g_keymap[AKEYCODE_NUMPAD_ADD] = IRR_KEY_ADD;
    g_keymap[AKEYCODE_NUMPAD_DOT] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_NUMPAD_COMMA] = IRR_KEY_COMMA;
    g_keymap[AKEYCODE_NUMPAD_ENTER] = IRR_KEY_RETURN;
    g_keymap[AKEYCODE_NUMPAD_EQUALS] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_NUMPAD_LEFT_PAREN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_NUMPAD_RIGHT_PAREN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_VOLUME_MUTE] = IRR_KEY_VOLUME_MUTE;
    g_keymap[AKEYCODE_INFO] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CHANNEL_UP] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CHANNEL_DOWN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_ZOOM_IN] = IRR_KEY_ZOOM;
    g_keymap[AKEYCODE_ZOOM_OUT] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_TV] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_WINDOW] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_GUIDE] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_DVR] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BOOKMARK] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CAPTIONS] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_SETTINGS] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_TV_POWER] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_TV_INPUT] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_STB_POWER] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_STB_INPUT] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_AVR_POWER] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_AVR_INPUT] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_PROG_RED] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_PROG_GREEN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_PROG_YELLOW] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_PROG_BLUE] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_APP_SWITCH] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_1] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_2] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_3] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_4] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_5] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_6] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_7] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_8] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_9] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_10] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_11] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_12] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_13] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_14] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_15] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BUTTON_16] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_LANGUAGE_SWITCH] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MANNER_MODE] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_3D_MODE] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CONTACTS] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CALENDAR] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MUSIC] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_CALCULATOR] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_ZENKAKU_HANKAKU] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_EISU] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MUHENKAN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_HENKAN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_KATAKANA_HIRAGANA] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_YEN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_RO] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_KANA] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_ASSIST] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BRIGHTNESS_DOWN] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_BRIGHTNESS_UP] = IRR_KEY_UNKNOWN;
    g_keymap[AKEYCODE_MEDIA_AUDIO_TRACK] = IRR_KEY_UNKNOWN;
}

void CIrrDeviceAndroid::openURL(const std::string& url)
{
    if (!Android)
        return;

    jobject native_activity = Android->activity->clazz;
    jclass class_native_activity = m_jni_env->GetObjectClass(native_activity);

    if (class_native_activity == NULL)
    {
        os::Printer::log("openURL unable to find object class.", ELL_ERROR);
        return;
    }

    jmethodID method_id = m_jni_env->GetMethodID(class_native_activity, "openURL", "(Ljava/lang/String;)V");

    if (method_id == NULL)
    {
        os::Printer::log("openURL unable to find method id.", ELL_ERROR);
        return;
    }
    jstring url_jstring = m_jni_env->NewStringUTF(url.c_str());
    m_jni_env->CallVoidMethod(native_activity, method_id, url_jstring);
}

void CIrrDeviceAndroid::toggleOnScreenKeyboard(bool show, s32 type)
{
    if (!Android)
        return;

    jobject native_activity = Android->activity->clazz;
    jclass class_native_activity = m_jni_env->GetObjectClass(native_activity);

    if (class_native_activity == NULL)
    {
        os::Printer::log("showKeyboard unable to find object class.", ELL_ERROR);
        return;
    }

    jmethodID method_id = NULL;
    if (show)
        method_id = m_jni_env->GetMethodID(class_native_activity, "showKeyboard", "(I)V");
    else
        method_id = m_jni_env->GetMethodID(class_native_activity, "hideKeyboard", "(Z)V");

    if (method_id == NULL)
    {
        os::Printer::log("showKeyboard unable to find method id.", ELL_ERROR);
        return;
    }

    if (show)
        m_jni_env->CallVoidMethod(native_activity, method_id, (jint)type);
    else
        m_jni_env->CallVoidMethod(native_activity, method_id, (jboolean)(type != 0));
}

void CIrrDeviceAndroid::fromSTKEditBox(int widget_id, const core::stringw& text, int selection_start, int selection_end, int type)
{
    if (!Android)
        return;

    jobject native_activity = Android->activity->clazz;
    jclass class_native_activity = m_jni_env->GetObjectClass(native_activity);

    if (class_native_activity == NULL)
    {
        os::Printer::log("fromSTKEditBox unable to find object class.", ELL_ERROR);
        return;
    }

    jmethodID method_id = m_jni_env->GetMethodID(class_native_activity, "fromSTKEditBox", "(ILjava/lang/String;III)V");
    if (method_id == NULL)
    {
        os::Printer::log("fromSTKEditBox unable to find method id.", ELL_ERROR);
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

    jstring jstring_text = m_jni_env->NewString((const jchar*)utf16.data(), utf16.size());

    m_jni_env->CallVoidMethod(native_activity, method_id, (jint)widget_id, jstring_text, (jint)selection_start, (jint)selection_end, (jint)type);
}

int CIrrDeviceAndroid::getRotation()
{
    jobject activity_obj = Android->activity->clazz;

    jclass activity = m_jni_env->GetObjectClass(activity_obj);
    jclass context = m_jni_env->FindClass("android/content/Context");
    jclass window_manager = m_jni_env->FindClass("android/view/WindowManager");
    jclass display = m_jni_env->FindClass("android/view/Display");

    jmethodID get_system_service = m_jni_env->GetMethodID(activity, "getSystemService",
                                            "(Ljava/lang/String;)Ljava/lang/Object;");
    jmethodID get_default_display = m_jni_env->GetMethodID(window_manager,
                                                   "getDefaultDisplay", 
                                                   "()Landroid/view/Display;");
    jmethodID get_rotation = m_jni_env->GetMethodID(display, "getRotation", "()I");

    jfieldID window_service = m_jni_env->GetStaticFieldID(context, "WINDOW_SERVICE",
                                                        "Ljava/lang/String;");

    jobject window_service_obj = m_jni_env->GetStaticObjectField(context, window_service);

    jobject window_manager_obj = m_jni_env->CallObjectMethod(activity_obj, get_system_service,
                                                   window_service_obj);

    jobject display_obj = m_jni_env->CallObjectMethod(window_manager_obj, get_default_display);
    
    int rotation = m_jni_env->CallIntMethod(display_obj, get_rotation);

    return rotation;
}

void CIrrDeviceAndroid::readApplicationInfo(ANativeActivity* activity)
{
    jobject activity_obj = activity->clazz;
    jclass activity_class = m_jni_env->GetObjectClass(activity_obj);

    jmethodID get_package_manager = m_jni_env->GetMethodID(activity_class,
                                       "getPackageManager", 
                                       "()Landroid/content/pm/PackageManager;");
    jobject package_manager_obj = m_jni_env->CallObjectMethod(activity_obj,
                                                        get_package_manager);

    jmethodID get_intent = m_jni_env->GetMethodID(activity_class, "getIntent",
                                            "()Landroid/content/Intent;");
                                            
    jobject intent_obj = m_jni_env->CallObjectMethod(activity_obj, get_intent);
    jclass intent = m_jni_env->FindClass("android/content/Intent");
    
    jmethodID get_component = m_jni_env->GetMethodID(intent, "getComponent",
                                           "()Landroid/content/ComponentName;");

    jobject component_name = m_jni_env->CallObjectMethod(intent_obj, get_component);

    jclass package_manager = m_jni_env->FindClass("android/content/pm/PackageManager");

    jfieldID get_meta_data_field = m_jni_env->GetStaticFieldID(package_manager,
                                                         "GET_META_DATA", "I");
    jint get_meta_data = m_jni_env->GetStaticIntField(package_manager,
                                                get_meta_data_field);

    jmethodID get_activity_info = m_jni_env->GetMethodID(package_manager,
                                           "getActivityInfo", 
                                           "(Landroid/content/ComponentName;I)"
                                           "Landroid/content/pm/ActivityInfo;");
         
    jobject activity_info_obj = m_jni_env->CallObjectMethod(package_manager_obj,
                                                      get_activity_info, 
                                                      component_name, 
                                                      get_meta_data);
    jclass activity_info = m_jni_env->FindClass("android/content/pm/ActivityInfo");
    
    jfieldID application_info_field = m_jni_env->GetFieldID(activity_info,
                                        "applicationInfo", 
                                        "Landroid/content/pm/ApplicationInfo;");
    jobject application_info_obj = m_jni_env->GetObjectField(activity_info_obj,
                                                       application_info_field);
    jclass application_info = m_jni_env->FindClass("android/content/pm/ApplicationInfo");
    
    jfieldID native_library_dir_field = m_jni_env->GetFieldID(application_info,
                                                        "nativeLibraryDir", 
                                                        "Ljava/lang/String;");
    jstring native_library_dir_str = (jstring)m_jni_env->GetObjectField(
                                                      application_info_obj, 
                                                      native_library_dir_field);
    const char* native_library_dir = m_jni_env->GetStringUTFChars(
                                                         native_library_dir_str, 
                                                         NULL);
    
    jfieldID data_dir_field = m_jni_env->GetFieldID(application_info, "dataDir",
                                             "Ljava/lang/String;");
    jstring data_dir_str = (jstring)m_jni_env->GetObjectField(application_info_obj,
                                                        data_dir_field);
    const char* data_dir = m_jni_env->GetStringUTFChars(data_dir_str, NULL);

    if (native_library_dir != NULL)
    {
        ApplicationInfo.native_lib_dir = native_library_dir;
    }
        
    if (data_dir != NULL)
    {
        ApplicationInfo.data_dir = data_dir;
    }
    
    ApplicationInfo.initialized = true;
}

const AndroidApplicationInfo& CIrrDeviceAndroid::getApplicationInfo(
                                                      ANativeActivity* activity)
{
    if (activity != NULL && ApplicationInfo.initialized == false)
    {
        readApplicationInfo(activity);
    }
    
    return ApplicationInfo;
}

DeviceOrientation CIrrDeviceAndroid::getDefaultOrientation()
{
    int rotation = getRotation();
    
    int32_t orientation = AConfiguration_getOrientation(Android->config);
    
    if (((rotation == 0 || rotation == 2) && 
        orientation == ACONFIGURATION_ORIENTATION_LAND) ||
        ((rotation == 1 || rotation == 3) && 
        orientation == ACONFIGURATION_ORIENTATION_PORT))
    {
        return ORIENTATION_LANDSCAPE;
    }
    else
    {
        return ORIENTATION_PORTRAIT;
    }
}

bool CIrrDeviceAndroid::activateAccelerometer(float updateInterval)
{
    if (!isAccelerometerAvailable())
        return false;
    
    if (DefaultOrientation == ORIENTATION_UNKNOWN)
    {
        DefaultOrientation = getDefaultOrientation();
    }

    int err = ASensorEventQueue_enableSensor(SensorEventQueue, Accelerometer);
    
    if (err == 0)
    {
        AccelerometerActive = true;
        
        ASensorEventQueue_setEventRate(SensorEventQueue, Accelerometer,
                    (int32_t)(updateInterval*1000.f*1000.f)); // in microseconds
                    
        os::Printer::log("Activated accelerometer", ELL_DEBUG);
    }

    return AccelerometerActive;
}

bool CIrrDeviceAndroid::deactivateAccelerometer()
{
    if (!Accelerometer)
        return false;

    int err = ASensorEventQueue_disableSensor(SensorEventQueue, Accelerometer);
    
    if (err == 0)
    {
        AccelerometerActive = false;
        os::Printer::log("Deactivated accelerometer", ELL_DEBUG);
    }
    
    return !AccelerometerActive;
}

bool CIrrDeviceAndroid::isAccelerometerActive()
{
    return AccelerometerActive;
}

bool CIrrDeviceAndroid::isAccelerometerAvailable()
{
    if (!Accelerometer)
    {
        Accelerometer = ASensorManager_getDefaultSensor(SensorManager,
                                                    ASENSOR_TYPE_ACCELEROMETER);
    }

    return (Accelerometer != NULL);
}

bool CIrrDeviceAndroid::activateGyroscope(float updateInterval)
{
    if (!isGyroscopeAvailable())
        return false;

    int err = ASensorEventQueue_enableSensor(SensorEventQueue, Gyroscope);
    
    if (err == 0)
    {
        GyroscopeActive = true;
        
        ASensorEventQueue_setEventRate(SensorEventQueue, Gyroscope,
                    (int32_t)(updateInterval*1000.f*1000.f)); // in microseconds

        os::Printer::log("Activated gyroscope", ELL_DEBUG);
    }
    
    return GyroscopeActive;
}

bool CIrrDeviceAndroid::deactivateGyroscope()
{
    if (!Gyroscope)
        return false;

    int err = ASensorEventQueue_disableSensor(SensorEventQueue, Gyroscope);
    
    if (err == 0)
    {
        GyroscopeActive = false;
        os::Printer::log("Deactivated gyroscope", ELL_DEBUG);
    }
    
    return !GyroscopeActive;
}

bool CIrrDeviceAndroid::isGyroscopeActive()
{
    return GyroscopeActive;
}

bool CIrrDeviceAndroid::isGyroscopeAvailable()
{
    if (!Gyroscope)
    {
        Gyroscope = ASensorManager_getDefaultSensor(SensorManager,
                                                    ASENSOR_TYPE_GYROSCOPE);
    }

    return (Gyroscope != NULL);
}

bool CIrrDeviceAndroid::hasHardwareKeyboard() const
{
    if (!Android)
        return false;

    jobject native_activity = Android->activity->clazz;
    jclass class_native_activity = m_jni_env->GetObjectClass(native_activity);

    if (class_native_activity == NULL)
    {
        os::Printer::log("isHardwareKeyboardConnected unable to find object class.", ELL_ERROR);
        return false;
    }

    jmethodID method_id = m_jni_env->GetMethodID(class_native_activity, "isHardwareKeyboardConnected", "()Z");

    if (method_id == NULL)
    {
        os::Printer::log("isHardwareKeyboardConnected unable to find method id.", ELL_ERROR);
        return false;
    }
    return m_jni_env->CallBooleanMethod(native_activity, method_id);
}

} // end namespace irr

#endif


