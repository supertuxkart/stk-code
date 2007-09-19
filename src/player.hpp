// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef TUXKART_PLAYER_H
#define TUXKART_PLAYER_H

#include <string>

enum AxisDirection { AD_NEGATIVE, AD_POSITIVE };

enum InputType { IT_NONE, IT_KEYBOARD, IT_STICKMOTION, IT_STICKBUTTON, IT_STICKHAT, IT_MOUSEMOTION, IT_MOUSEBUTTON };
#define IT_LAST (IT_MOUSEBUTTON)

typedef struct
{
    InputType type;
    int id0;
    int id1;
    int id2;
}
Input;

// Some part (e.g. gui/PlayerControls) depend on KC_LEFT being the first
// entry, and KC_FIRE being the last - so any action added should be
// added in between those two values.
enum KartActions { KC_LEFT,
                   KC_RIGHT,
                   KC_ACCEL,
                   KC_BRAKE,
                   KC_WHEELIE,
                   KC_JUMP,
                   KC_RESCUE,
                   KC_FIRE,
                   KC_LOOK_BACK };
#define KC_LAST (KC_LOOK_BACK)

extern const char *sKartAction2String[KC_LAST+1];
/*class for managing player name and control configuration*/
class Player
{
private:
    std::string m_name;
    Input m_action_map[KC_LAST+1];
    unsigned int m_last_kart_id;

public:
    Player(){}
    Player(const std::string &name_):m_name(name_){}
    void setName(const std::string &name_){m_name = name_;}

    void setKey(KartActions action, int key) {}
    void setButton(KartActions action, int button){ }

    std::string getName() {return m_name;}

    Input *getInput(KartActions action) { return &m_action_map[action]; }
    void setInput(KartActions action, InputType type, int id0, int id1, int id2)
    {
        Input *i = &m_action_map[action];
        i->type = type;
        i->id0 = id0;
        i->id1 = id1;
        i->id2 = id2;
    }

    unsigned int getLastKartId(){ return m_last_kart_id; }
    void setLastKartId(int newLastKartId){ m_last_kart_id = newLastKartId; }
};

#endif

/*EOF*/
