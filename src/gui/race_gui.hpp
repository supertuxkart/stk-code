//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_RACEGUI_H
#define HEADER_RACEGUI_H

#include "base_gui.hpp"
#include "material.hpp"
#include "player_kart.hpp"
#include "player.hpp"

// TODO: Fix this.
#define MAX_ID0 512
#define MAX_ID1 16
#define MAX_ID2 2

typedef struct
{
    PlayerKart *kart;
    KartActions action;
}
Entry;

class RaceSetup;

class RaceGUI: public BaseGUI
{
    // A mapping for the assigned keys (like fire, ...) to
    // the kart which is using them
    Entry m_input_map[IT_LAST+1][MAX_ID0][MAX_ID1][MAX_ID2];

    float       m_time_of_leader;
    int         m_lap_leader;

public:
    RaceGUI();
    ~RaceGUI();
    void update(float dt);
    void select() {}
    void input(InputType type, int id0, int id1, int id2, int value);
    void handleKartAction(KartActions ka, int value);

private:
    ulClock   m_fps_timer;
    int       m_fps_counter;
    char      m_fps_string[10];
    double    m_time_left ;
    char*     m_pos_string [11];
    Material* m_steering_wheel_icon;
    Material* m_speed_back_icon;
    Material* m_speed_fore_icon;

    /* Display informat on screen */
    void drawStatusText        (const RaceSetup& raceSetup, const float dt);
    void drawEnergyMeter       (Kart *player_kart,
                                int   offset_x, int   offset_y,
                                float ratio_x,  float ratio_y  );
    void drawCollectableIcons  (Kart* player_kart,
                                int   offset_x, int   offset_y,
                                float ratio_x,  float ratio_y  );
    void drawEmergencyText     (Kart* player_kart,
                                int   offset_x, int   offset_y,
                                float ratio_x,  float ratio_y  );
    void UpdateKeyboardMappings();
    void putEntry(PlayerKart *kart, KartActions ka);
    bool handleInput(InputType type, int id0, int id1, int id2, int value);
    void inputKeyboard(int key, int pressed);
    void drawPlayerIcons       ();
    void oldDrawPlayerIcons    ();
    void drawGameOverText      (const float dt);
    void drawMap               ();
    void drawTimer             ();
    void drawFPS               ();

    /* Text drawing */
    /** Draw text to screen.
        scale_x and scale_y could be used to a simple resize (e.g. for multiplayer
        split screens, though, currently, we reduce fonts size to half).        */
    void drawSteering             (Kart* kart, int offset_x, int offset_y,
                                   float ratio_x, float ratio_y           );
    void drawPosition             (Kart* kart, int offset_x, int offset_y,
                                   float ratio_x, float ratio_y           );
    void drawSpeed                (Kart* kart, int offset_x, int offset_y,
                                   float ratio_x, float ratio_y           );
    void drawLap                  (Kart* kart, int offset_x, int offset_y,
                                   float ratio_x, float ratio_y           );
#if 0
    void drawTexture              (const GLuint texture, int w, int h, int red,
                                   int green, int blue, int x, int y);
#endif
};

#endif
