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

#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"

using namespace irr;

namespace GUIEngine
{
// global instance of the current dialog if any
static ModalDialog* modalWindow = NULL;

ModalDialog::ModalDialog(const float percentWidth, const float percentHeight)
{
#if IRRLICHT_VERSION_MAJOR > 1 || IRRLICHT_VERSION_MINOR >= 6
    const core::dimension2d<u32>& frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
#else
    const core::dimension2d<s32>& frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
#endif

    const int w = (int)(frame_size.Width*percentWidth);
    const int h = (int)(frame_size.Height*percentHeight);
    
    assert(frame_size.Width > 0);
    assert(frame_size.Height > 0);
    assert(frame_size.Width < 99999);
    assert(frame_size.Height < 99999);
    
    assert(w > 0);
    assert(h > 0);
    assert((unsigned int)w <= frame_size.Width);
    assert((unsigned int)h <= frame_size.Height);
    
    m_area = core::rect< s32 >( position2d< s32 >(frame_size.Width/2 - w/2, frame_size.Height/2 - h/2),
                               dimension2d< s32 >(w, h) );
    
    if (modalWindow != NULL) delete modalWindow;
    modalWindow = this;
    
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow  	( m_area, true /* modal */ );
}


ModalDialog::~ModalDialog()
{
    // irrLicht is to stupid to remove focus from deleted widgets
    // so do it by hand
    GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
    
    m_irrlicht_window->remove();
    
    if(modalWindow == this) modalWindow = NULL;
}

void ModalDialog::clearWindow()
{
    const int children_amount = m_children.size();
    for(int i=0; i<children_amount; i++)
    {
        m_irrlicht_window->removeChild( m_children[i].getIrrlichtElement() );
    }
    m_children.clearAndDeleteAll();   
    
    m_irrlicht_window->remove();
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow  	( m_area, true /* modal */ );
    
    /*
    const core::list<IGUIElement*>& remainingChildren = m_irrlicht_window->getChildren();
    const int amount = remainingChildren.getSize();
    for(core::list<IGUIElement*>::Iterator it=remainingChildren.begin(); it != remainingChildren.end(); it++)
    {
        it->remove();
    }
     */
}
    
void ModalDialog::dismiss()
{
    if(modalWindow != NULL) delete modalWindow;
    modalWindow = NULL;
}

void ModalDialog::onEnterPressed()
{
    if(modalWindow != NULL) modalWindow->onEnterPressedInternal();
}

bool ModalDialog::isADialogActive()
{
    return modalWindow != NULL;
}
ModalDialog* ModalDialog::getCurrent()
{
    return modalWindow;
}

void ModalDialog::onEnterPressedInternal()
{
}
    
}
