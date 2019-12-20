// Copyright (C) 2002-2008 Nikolaus Gebhardt
// Copyright (C) 2008 Redshift Software, Inc.
// Copyright (C) 2012-2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_IOS_H_INCLUDED__
#define __C_IRR_DEVICE_IOS_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_IOS_DEVICE_

#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"
#include <map>
#include <set>
#include <string>

namespace irr
{
    class CIrrDeviceiOS : public CIrrDeviceStub, public video::IImagePresenter
    {
    public:
        CIrrDeviceiOS(const SIrrlichtCreationParameters& params);
        void swapBuffers();
        void beginScene();
        virtual ~CIrrDeviceiOS();

        virtual bool run();
        virtual void yield();
        virtual void sleep(u32 timeMs, bool pauseTimer);

        virtual void setWindowCaption(const wchar_t* text);
        virtual void setWindowClass(const char* text) {}
        virtual bool moveWindow(int x, int y) { return false; }
        virtual bool isWindowActive() const;
        virtual bool isWindowFocused() const;
        virtual bool isWindowMinimized() const;

        virtual bool present(video::IImage* surface, void * windowId = 0, core::rect<s32>* src = 0);

        virtual void closeDevice();

        virtual void setResizable(bool resize = false);

        virtual void minimizeWindow();
        virtual void maximizeWindow();
        virtual void restoreWindow();

        virtual bool getWindowPosition(int* x, int* y);

        virtual bool activateAccelerometer(float updateInterval);
        virtual bool deactivateAccelerometer();
        virtual bool isAccelerometerActive();
        virtual bool isAccelerometerAvailable();
        virtual bool activateGyroscope(float updateInterval);
        virtual bool deactivateGyroscope();
        virtual bool isGyroscopeActive();
        virtual bool isGyroscopeAvailable();
        virtual bool activateDeviceMotion(float updateInterval) ;
        virtual bool deactivateDeviceMotion() ;
        virtual bool isDeviceMotionActive();
        virtual bool isDeviceMotionAvailable();
        virtual E_DEVICE_TYPE getType() const;
        virtual u32 getScreenHeight() const { return 0; }
        virtual u32 getOnScreenKeyboardHeight() const { return 0; }
        virtual s32 getMovedHeight() const { return 0; }
        virtual void toggleOnScreenKeyboard(bool show, s32 type = 0) {}
        virtual void registerGetMovedHeightFunction(HeightFunc) {}

        //! Returns true if system has touch device
        virtual bool supportsTouchDevice() const { return true; }

        //! Returns true if system has hardware keyboard attached
        virtual bool hasHardwareKeyboard() const { return false; }

        //! Returns true if system has native on screen keyboard
        virtual bool hasOnScreenKeyboard() const  { return false; }

        //! Get a unique touch id per touch, create one if it's a new touch
        size_t getTouchId(void* touch)
        {
            auto it = m_touch_id_map.find(touch);
            if (it == m_touch_id_map.end())
            {
                std::set<size_t> ids;
                for (auto& p : m_touch_id_map)
                    ids.insert(p.second);
                size_t cur_id = 0;
                while (true)
                {
                    if (ids.find(cur_id) == ids.end())
                        break;
                    cur_id++;
                }
                m_touch_id_map[touch] = cur_id;
                return cur_id;
            }
            return it->second;
        }

        //! Remove a unique touch id, free it for future usage
        void removeTouchId(void* touch)
        {
            m_touch_id_map.erase(touch);
        }

        //! Clear all unique touch ids, used when the app out focused
        void clearAllTouchIds()
        {
            m_touch_id_map.clear();
        }
        void setUpsideDown(bool val) { m_upside_down = val; }
        void setPaddings(float top, float bottom, float left, float right)
        {
            m_top_padding = top;
            m_bottom_padding = bottom;
            m_left_padding = left;
            m_right_padding = right;
        }
        static std::string getSystemLanguageCode();
        static void openURLiOS(const char* url);
        virtual s32 getTopPadding()
        {
            return m_top_padding * m_native_scale;
        }
        virtual s32 getBottomPadding()
        {
            return m_bottom_padding * m_native_scale;
        }
        virtual s32 getLeftPadding()
        {
            return m_left_padding * m_native_scale;
        }
        virtual s32 getRightPadding()
        {
            return m_right_padding * m_native_scale;
        }
    private:
        void createWindow();
        void createViewAndDriver();

        void* DataStorage;

        bool Close;

        std::map<void*, size_t> m_touch_id_map;
        bool m_upside_down;
        float m_top_padding;
        float m_bottom_padding;
        float m_left_padding;
        float m_right_padding;
        float m_native_scale;
    };

}

#define _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
extern int ios_main(int argc, char *argv[]);
extern void override_default_params_for_mobile();
#endif

#endif
#endif
