// Copyright (C) 2002-2011 Nikolaus Gebhardt
// Copyright (C) 2016-2017 Dawid Gan
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_ANDROID_H_INCLUDED__
#define __C_IRR_DEVICE_ANDROID_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include <android/window.h>
#include <android/sensor.h>
#include "stk_android_native_app_glue.h"
#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"

#include <map>
#include <string>

namespace irr
{
    enum DeviceOrientation
    {
        ORIENTATION_UNKNOWN,
        ORIENTATION_PORTRAIT,
        ORIENTATION_LANDSCAPE
    };
    
    struct AndroidApplicationInfo
    {
        std::string native_lib_dir;
        std::string data_dir;
        bool initialized;
        
        AndroidApplicationInfo() : initialized(false) {};
    };
    
    class CIrrDeviceAndroid : public CIrrDeviceStub, video::IImagePresenter
    {
    public:
        //! constructor
        CIrrDeviceAndroid(const SIrrlichtCreationParameters& param);

        //! destructor
        virtual ~CIrrDeviceAndroid();

        virtual bool run();
        virtual void yield();
        virtual void sleep(u32 timeMs, bool pauseTimer=false);
        virtual void setWindowCaption(const wchar_t* text);
        virtual void setWindowClass(const char* text) {}
        virtual bool present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip);
        virtual bool isWindowActive() const;
        virtual bool isWindowFocused() const;
        virtual bool isWindowMinimized() const;
        virtual void closeDevice();
        virtual void setResizable( bool resize=false );
        virtual void minimizeWindow();
        virtual void maximizeWindow();
        virtual void restoreWindow();
        virtual bool moveWindow(int x, int y);
        virtual bool getWindowPosition(int* x, int* y);
        virtual E_DEVICE_TYPE getType() const;
        virtual bool activateAccelerometer(float updateInterval);
        virtual bool deactivateAccelerometer();
        virtual bool isAccelerometerActive();
        virtual bool isAccelerometerAvailable();
        virtual bool activateGyroscope(float updateInterval);
        virtual bool deactivateGyroscope();
        virtual bool isGyroscopeActive();
        virtual bool isGyroscopeAvailable();
        virtual void setTextInputEnabled(bool enabled) {TextInputEnabled = enabled;}
        void fromSTKEditBox(int widget_id, const core::stringw& text, int selection_start, int selection_end, int type);
        virtual void toggleOnScreenKeyboard(bool show, s32 type = 0);
        virtual bool supportsTouchDevice() const { return HasTouchDevice; }
        virtual bool hasHardwareKeyboard() const;
        // ATM if there is touch device we assume native screen keyboard is
        // available which for example android tv doesn't
        virtual bool hasOnScreenKeyboard() const { return HasTouchDevice; }
        virtual u32 getScreenHeight() const { return m_screen_height; }
        virtual u32 getOnScreenKeyboardHeight() const;
        virtual s32 getMovedHeight() const { return m_moved_height; }
        virtual void registerGetMovedHeightFunction(HeightFunc height_function)
        {
            m_moved_height_func = height_function;
        }
        static void onCreate();
        static const AndroidApplicationInfo& getApplicationInfo(
                                                    ANativeActivity* activity);
        void openURL(const std::string& url);
    private:
        s32 m_moved_height;
        u32 m_screen_height;
        HeightFunc m_moved_height_func;
        android_app* Android;
        ASensorManager* SensorManager;
        ASensorEventQueue* SensorEventQueue;
        const ASensor* Accelerometer;
        const ASensor* Gyroscope;
        bool AccelerometerActive;
        bool GyroscopeActive;
        bool TextInputEnabled;
        static AndroidApplicationInfo ApplicationInfo;

        static bool IsPaused;
        static bool IsFocused;
        static bool IsStarted;
        
        struct TouchEventData
        {
            int x;
            int y;
            ETOUCH_INPUT_EVENT event;
            
            TouchEventData() : x(0), y(0), event(ETIE_COUNT) {};
        };
        
        TouchEventData TouchEventsData[32];
        bool HasTouchDevice;
        float GamepadAxisX;
        float GamepadAxisY;
        DeviceOrientation DefaultOrientation;

        video::SExposedVideoData ExposedVideoData;

        std::map<int, EKEY_CODE> KeyMap;
        
        void printConfig();
        void createDriver();
        void createKeyMap();
        void createVideoModeList();
        wchar_t getKeyChar(SEvent& event);
        wchar_t getUnicodeChar(AInputEvent* event);
        static void readApplicationInfo(ANativeActivity* activity);
        int getRotation();
        DeviceOrientation getDefaultOrientation();
        video::SExposedVideoData& getExposedVideoData();
        
        static void handleAndroidCommand(android_app* app, int32_t cmd);
        static s32 handleInput(android_app* app, AInputEvent* event);

        s32 handleTouch(AInputEvent* androidEvent);
        s32 handleKeyboard(AInputEvent* androidEvent);
        s32 handleGamepad(AInputEvent* androidEvent);
    };

} // end namespace irr

#endif // _IRR_COMPILE_WITH_ANDROID_DEVICE_
#endif // __C_IRR_DEVICE_ANDROID_H_INCLUDED__
