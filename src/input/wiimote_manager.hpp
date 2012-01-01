//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 SuperTuxKart-Team
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

#ifndef WIIMOTE_MANAGER_HPP
#define WIIMOTE_MANAGER_HPP

#ifdef ENABLE_WIIUSE

extern const int    MAX_WIIMOTES;
extern const int    WIIMOTE_AXES;
extern const int    WIIMOTE_BUTTONS;

struct wiimote_t;

class WiimoteManager
{
private:
    wiimote_t**     m_wiimotes;
    int             m_nb_wiimotes;

public:
    WiimoteManager();
    ~WiimoteManager();
    
    void launchDetection(int timeout);
    void update();
    void cleanup();
    
    int getNbWiimotes() const   {return m_nb_wiimotes;}
};

extern WiimoteManager* wiimote_manager;

#endif

#endif // WIIMOTE_MANAGER_HPP
