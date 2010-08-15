//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
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
#ifdef ADDONS_MANAGER

#ifndef HEADER_ADDONS_UPDATE_SCREEN_HPP
#define HEADER_ADDONS_UPDATE_SCREEN_HPP

#include "guiengine/screen.hpp"

class Addons;
class AddonsLoading;

namespace GUIEngine { class Widget; }

/**
  * \brief Help screen, part 1
  * \ingroup states_screens
  */
class AddonsUpdateScreen : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<AddonsUpdateScreen>
{
    friend class GUIEngine::ScreenSingleton<AddonsUpdateScreen>;
    AddonsUpdateScreen();
    Addons * addons;
    /*uneeded*/
    AddonsLoading  * load;
    void loadInformations();
    
public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init();
    };
#endif
#endif
