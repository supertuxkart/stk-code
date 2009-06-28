//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "irrlicht.h"
#include "utils/ptr_vector.hpp"

class Player;

namespace GUIEngine
{
    class Widget;
    class TextBoxWidget;
    class ButtonWidget;
    
/**
  * Base class, derive your own.
  * Only once instance at a time (if you create a 2nd the first will be destroyed).
  * You can call static function 'dismiss' to simply close current dialog (so you don't
  * need to keep track of instances yourself)
  */
class ModalDialog
{
protected:
    irr::gui::IGUIWindow* m_irrlicht_window;
    core::rect< s32 > m_area;
    
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    ModalDialog(const float percentWidth, const float percentHeight);
    
    virtual void onEnterPressedInternal();
    
public:
    ptr_vector<Widget> m_children;
    
    virtual ~ModalDialog();
    virtual void processEvent(std::string& eventSource){}
    
    static void dismiss();
    static void onEnterPressed();
    static ModalDialog* getCurrent();
    static bool isADialogActive();
};

class PressAKeyDialog : public ModalDialog
{
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    PressAKeyDialog(const float percentWidth, const float percentHeight);
    void processEvent(std::string& eventSource);
};

class EnterPlayerNameDialog : public ModalDialog
{
    TextBoxWidget* textCtrl;
    ButtonWidget* cancelButton;
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    EnterPlayerNameDialog(const float percentWidth, const float percentHeight);
    ~EnterPlayerNameDialog();

    void onEnterPressedInternal();
    void processEvent(std::string& eventSource);
};

class TrackInfoDialog : public ModalDialog
{
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    TrackInfoDialog(const char* trackName, irr::video::ITexture* screenshot, const float percentWidth, const float percentHeight);    
    void onEnterPressedInternal();
};

class PlayerInfoDialog : public ModalDialog
{
    TextBoxWidget* textCtrl;
    Player* m_player;
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    PlayerInfoDialog(Player* PlayerInfoDialog,
                     const float percentWidth, const float percentHeight);
    void onEnterPressedInternal();
    void processEvent(std::string& eventSource);
};
    
    
}
