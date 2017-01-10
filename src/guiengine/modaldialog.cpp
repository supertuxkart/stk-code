//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "guiengine/modaldialog.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/layout_manager.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <IGUIEnvironment.h>
#include <IGUIButton.h>

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

ModalDialog::ModalDialog(const float percentWidth, const float percentHeight,
                         ModalDialogLocation location)
{
    m_dialog_location = location;
    m_init            = false;
    m_fade_background = true;
    m_percent_width   = percentWidth;
    m_percent_height  = percentHeight;
    m_irrlicht_window = NULL;
}   // ModalDialog

// ----------------------------------------------------------------------------

void ModalDialog::loadFromFile(const char* xmlFile)
{
    doInit();
    std::string path = file_manager->getAssetChecked(FileManager::GUI,xmlFile,
                                                     true);
    IXMLReader* xml = file_manager->createXMLReader(path);

    Screen::parseScreenFileDiv(xml, m_widgets, m_irrlicht_window);
    delete xml;

    loadedFromFile();

    LayoutManager::calculateLayout( m_widgets, this );

    beforeAddingWidgets();

    addWidgetsRecursively(m_widgets);

    init();
}   // loadFromFile

// ----------------------------------------------------------------------------

void ModalDialog::doInit()
{
    if(m_init) return;
    m_init = true;
    pointer_was_shown = irr_driver->isPointerShown();
    irr_driver->showPointer();

    const core::dimension2d<u32>& frame_size =
        GUIEngine::getDriver()->getCurrentRenderTargetSize();

    const int w = (int)(frame_size.Width* m_percent_width);
    const int h = (int)(frame_size.Height* m_percent_height);

    assert(frame_size.Width > 0);
    assert(frame_size.Height > 0);
    assert(frame_size.Width < 99999);
    assert(frame_size.Height < 99999);

    assert(w > 0);
    assert(h > 0);

    assert((unsigned int)w <= frame_size.Width);
    assert((unsigned int)h <= frame_size.Height);

    if (m_dialog_location == MODAL_DIALOG_LOCATION_CENTER)
    {
        m_area = core::rect<s32>(core::position2d<s32>(frame_size.Width/2 - w/2,
                                                       frame_size.Height/2 - h/2),
                                 core::dimension2d<s32>(w, h));
    }
    else if (m_dialog_location == MODAL_DIALOG_LOCATION_BOTTOM)
    {
        m_area = core::rect<s32>(core::position2d<s32>(frame_size.Width/2 - w/2,
                                                       frame_size.Height - h - 15),
                                 core::dimension2d<s32>(w, h));
    }
    else
    {
        assert(false);
    }

    if (modalWindow != NULL)
    {
        delete modalWindow;
        Log::warn("GUIEngine", "Showing a modal dialog while the previous one "
                  "is still open. Destroying the previous dialog.");
    }
    modalWindow = this;

    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow(m_area,
                                                          true /* modal */);
    m_irrlicht_window->setDrawTitlebar(false);
    m_irrlicht_window->getCloseButton()->setVisible(false);
    if (!UserConfigParams::m_artist_debug_mode)
        m_irrlicht_window->setDraggable(false);

    GUIEngine::getSkin()->m_dialog = true;
    GUIEngine::getSkin()->m_dialog_size = 0.0f;

    m_previous_mode=input_manager->getMode();
    input_manager->setMode(InputManager::MENU);
}   // doInit

// ----------------------------------------------------------------------------

ModalDialog::~ModalDialog()
{
    GUIEngine::getSkin()->m_dialog = false;
    GUIEngine::getSkin()->m_dialog_size = 0.0f;

    // irrLicht is to stupid to remove focus from deleted widgets
    // so do it by hand
    GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );

    m_irrlicht_window->remove();
    m_irrlicht_window = NULL;

    if (modalWindow == this) modalWindow = NULL;

    // restore previous pointer state
    if (pointer_was_shown)  irr_driver->showPointer();
    else                    irr_driver->hidePointer();

    input_manager->setMode(m_previous_mode);


    // it's generally not necessay to do that because references
    // to the deleted widgets will be gone, but some widgets
    // may want to perform additional cleanup at this time
    elementsWereDeleted();
}   // ~ModalDialog

// ----------------------------------------------------------------------------

void ModalDialog::clearWindow()
{
    assert(m_irrlicht_window != NULL);

    Widget* w;
    for_in (w, m_widgets)
    {
        m_irrlicht_window->removeChild( w->getIrrlichtElement() );
    }
    elementsWereDeleted();
    m_widgets.clearAndDeleteAll();

    m_irrlicht_window->remove();
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow( m_area, true /* modal */ );
}   // clearWindow

// ----------------------------------------------------------------------------

void ModalDialog::dismiss()
{
    if(modalWindow != NULL) delete modalWindow;
    modalWindow = NULL;
    if(GUIEngine::getCurrentScreen() != NULL)
        GUIEngine::getCurrentScreen()->onDialogClose();
}   // dismiss

// ----------------------------------------------------------------------------

void ModalDialog::onEnterPressed()
{
    if(modalWindow != NULL) modalWindow->onEnterPressedInternal();
}   // onEnterPressed

// ----------------------------------------------------------------------------

bool ModalDialog::isADialogActive()
{
    return modalWindow != NULL;
}   // isADialogActive

// ----------------------------------------------------------------------------

ModalDialog* ModalDialog::getCurrent()
{
    return modalWindow;
}   // getCurrent

// ----------------------------------------------------------------------------

void ModalDialog::onEnterPressedInternal()
{
}   // onEnterPressedInternal

// ----------------------------------------------------------------------------


