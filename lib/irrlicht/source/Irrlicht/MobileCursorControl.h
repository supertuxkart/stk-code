#ifndef __C_MOBILE_CURSOR_CONTROL_H_INCLUDED__
#define __C_MOBILE_CURSOR_CONTROL_H_INCLUDED__
#include "ICursorControl.h"

namespace irr
{
namespace gui
{
    class MobileCursorControl : public ICursorControl
    {
    public:

        MobileCursorControl() : CursorPos(core::position2d<s32>(0, 0)) {}
        virtual void setVisible(bool visible) {}
        virtual bool isVisible() const {return false;}
        virtual void setPosition(const core::position2d<f32> &pos)
        {
            setPosition(pos.X, pos.Y);
        }
        virtual void setPosition(f32 x, f32 y)
        {
            CursorPos.X = x;
            CursorPos.Y = y;
        }
        virtual void setPosition(const core::position2d<s32> &pos)
        {
            setPosition(pos.X, pos.Y);
        }
        virtual void setPosition(s32 x, s32 y)
        {
            CursorPos.X = x;
            CursorPos.Y = y;
        }
        virtual const core::position2d<s32>& getPosition()
        {
            return CursorPos;
        }
        virtual core::position2d<f32> getRelativePosition()
        {
            return core::position2d<f32>(0, 0);
        }
        virtual void setReferenceRect(core::rect<s32>* rect=0) {}
    private:
        core::position2d<s32> CursorPos;
    };
}
}
#endif
