//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "states_screens/dialogs/registration_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/register_screen.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

RegistrationDialog::RegistrationDialog() :
        ModalDialog(0.8f,0.9f)
{
    loadFromFile("online/registration_terms.stkgui");
    LabelWidget* terms_widget = getWidget<LabelWidget>("terms");

    core::stringw terms = _("Please read the terms and conditions "
        "for SuperTuxKart at '%s'. You must agree "
        "to these terms in order to register an account for STK. "
        "If you have any questions or comments regarding these "
        "terms, one of the members of the development team would gladly "
        "assist you.",
        "https://terms.supertuxkart.net");
    terms_widget->setText(terms, false);

   // showRegistrationTerms();
}

// -----------------------------------------------------------------------------

RegistrationDialog::~RegistrationDialog()
{
}

// -----------------------------------------------------------------------------
/** Process input events.
 *  \event_source name of the widget that triggered the event.
 */
EventPropagation RegistrationDialog::processEvent(const std::string& event_source)
{
    if (event_source == "options")
    {
        RibbonWidget *rib = getWidget<RibbonWidget>("options");
        std::string s = rib->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if(s=="accept")
        {
            GUIEngine::Screen *s = GUIEngine::getCurrentScreen();
            RegisterScreen *r = dynamic_cast<RegisterScreen*>(s);
            assert(r);
            r->acceptTerms();
        }

        // If it's not accept, it's cancel - anyway, close dialog
        ModalDialog::dismiss();
        return EVENT_BLOCK;
    }
    return EVENT_LET;
}   // processEvent
