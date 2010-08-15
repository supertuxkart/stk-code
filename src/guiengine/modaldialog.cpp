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

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/layout_manager.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"

#include "irrlicht.h"
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace GUIEngine
{
    /** global instance of the current dialog if any */
    static ModalDialog* modalWindow = NULL;

    /** To remember and restore the previous state */
    bool pointer_was_shown;
}

using namespace GUIEngine;

// ----------------------------------------------------------------------------

ModalDialog::ModalDialog(const float percentWidth, const float percentHeight)
{
    doInit(percentWidth, percentHeight);
}

// ----------------------------------------------------------------------------

void traceChildren(const ptr_vector<Widget>& widgets, int indent=0)
{
    for (int n=0; n<widgets.size(); n++)
    {
        for (int i=0; i<indent; i++) std::cout << "    ";
        std::cout << "    Type " << widgets[n].getType() << " : "
                  << const_cast<Widget*>(widgets.getConst(n))->m_properties[PROP_ID].c_str() << "\n";
        traceChildren (widgets[n].getChildren(), indent+1);
    }
}

ModalDialog::ModalDialog(const char* xmlFile, const float percentWidth, const float percentHeight)
{
    // FIXME: dialog are destroyed when dismissed, this means the disk
    // will be seeked everytime the same dialog is opened to read its
    // XML file... cache loaded dialogs somehow maybe?
    doInit(percentWidth, percentHeight);
    IrrXMLReader* xml = irr::io::createIrrXMLReader( (file_manager->getGUIDir() + "/" + xmlFile).c_str() );
    Screen::parseScreenFileDiv(xml, m_children, m_irrlicht_window);
    delete xml;

    std::cout << "Dialog children :\n";
    traceChildren(m_children);
    
    LayoutManager::calculateLayout( m_children, this );
    
    addWidgetsRecursively(m_children);
}

// ----------------------------------------------------------------------------

void ModalDialog::doInit(const float percentWidth, const float percentHeight)
{
    pointer_was_shown = irr_driver->isPointerShown();
    irr_driver->showPointer();
    
    const core::dimension2d<u32>& frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
    
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
    
    m_area = core::rect< s32 >( core::position2d< s32 >(frame_size.Width/2 - w/2, frame_size.Height/2 - h/2),
                               core::dimension2d< s32 >(w, h) );
    
    if (modalWindow != NULL) delete modalWindow;
    modalWindow = this;
    
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow   ( m_area, true /* modal */ );
    
    GUIEngine::getSkin()->m_dialog = true;
    GUIEngine::getSkin()->m_dialog_size = 0.0f;
    
    m_previous_mode=input_manager->getMode();
    input_manager->setMode(InputManager::MENU);
}

// ----------------------------------------------------------------------------

ModalDialog::~ModalDialog()
{
    GUIEngine::getSkin()->m_dialog = false;
    GUIEngine::getSkin()->m_dialog_size = 0.0f;

    // irrLicht is to stupid to remove focus from deleted widgets
    // so do it by hand
    GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
    
    m_irrlicht_window->remove();
    
    if (modalWindow == this) modalWindow = NULL;
    
    // restore previous pointer state
    if (pointer_was_shown)  irr_driver->showPointer();
    else                    irr_driver->hidePointer();

    input_manager->setMode(m_previous_mode);
}

// ----------------------------------------------------------------------------

void ModalDialog::clearWindow()
{
    const int children_amount = m_children.size();
    for(int i=0; i<children_amount; i++)
    {
        m_irrlicht_window->removeChild( m_children[i].getIrrlichtElement() );
    }
    m_children.clearAndDeleteAll();   
    
    m_irrlicht_window->remove();
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow   ( m_area, true /* modal */ );
    
    /*
    const core::list<IGUIElement*>& remainingChildren = m_irrlicht_window->getChildren();
    const int amount = remainingChildren.getSize();
    for(core::list<IGUIElement*>::Iterator it=remainingChildren.begin(); it != remainingChildren.end(); it++)
    {
        it->remove();
    }
     */
}

// ----------------------------------------------------------------------------
    
void ModalDialog::dismiss()
{
    if(modalWindow != NULL) delete modalWindow;
    modalWindow = NULL;
}

// ----------------------------------------------------------------------------

void ModalDialog::onEnterPressed()
{
    if(modalWindow != NULL) modalWindow->onEnterPressedInternal();
}

// ----------------------------------------------------------------------------

bool ModalDialog::isADialogActive()
{
    return modalWindow != NULL;
}

// ----------------------------------------------------------------------------

ModalDialog* ModalDialog::getCurrent()
{
    return modalWindow;
}

// ----------------------------------------------------------------------------

void ModalDialog::onEnterPressedInternal()
{
}

// ----------------------------------------------------------------------------

Widget* ModalDialog::getLastWidget()
{
    // FIXME: don't duplicate this code from 'Screen.cpp'
    const int childrenCount = m_children.size();
    
    for (int i=childrenCount-1; i>=0; i--)
    {
        if (m_children[i].getIrrlichtElement() == NULL ||
            m_children[i].getIrrlichtElement()->getTabOrder() == -1 ||
            m_children[i].getIrrlichtElement()->getTabOrder() >= 1000 /* non-tabbing items are given such IDs */ ||
            !m_children[i].m_focusable)
        {
            continue;
        }
        return m_children.get(i);

    }
    return NULL;
}

// ----------------------------------------------------------------------------

Widget* ModalDialog::getFirstWidget()
{
    // FIXME: don't duplicate this code from 'Screen.cpp'
    const int childrenCount = m_children.size();
    
    for (int i=0; i<childrenCount; i++)
    {
        if (m_children[i].getIrrlichtElement() == NULL ||
            m_children[i].getIrrlichtElement()->getTabOrder() == -1 ||
            m_children[i].getIrrlichtElement()->getTabOrder() >= 1000 /* non-tabbing items are given such IDs */ ||
            !m_children[i].m_focusable)
        {
            continue;
        }
        return m_children.get(i);
    }
    return NULL;
}
    
// ----------------------------------------------------------------------------

// FIXME: this code was duplicated from Screen, find a way to share instead of duplicating
void ModalDialog::addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent)
{
    const unsigned short widgets_amount = widgets.size();
    
    // ------- add widgets
    for (int n=0; n<widgets_amount; n++)
    {
        if (widgets[n].getType() == WTYPE_DIV)
        {
            widgets[n].add(); // Will do nothing, but will maybe reserve an ID
            addWidgetsRecursively(widgets[n].m_children, &widgets[n]);
        }
        else
        {
            // warn if widget has no dimensions (except for ribbons and icons, where it is normal since it
            // adjusts to its contents)
            if ((widgets[n].m_w < 1 || widgets[n].m_h < 1) &&
                widgets[n].getType() != WTYPE_RIBBON &&
                widgets[n].getType() != WTYPE_ICON_BUTTON)
            {
                std::cerr << "/!\\ Warning /!\\ : widget " << widgets[n].m_properties[PROP_ID].c_str() << " has no dimensions" << std::endl;
            }
            
            if (widgets[n].m_x == -1 || widgets[n].m_y == -1)
            {
                std::cerr << "/!\\ Warning /!\\ : widget " << widgets[n].m_properties[PROP_ID].c_str() << " has no position" << std::endl;
            }
            
            widgets[n].add();
        }
        
    } // next widget
    
}   // addWidgetsRecursively

// ----------------------------------------------------------------------------
