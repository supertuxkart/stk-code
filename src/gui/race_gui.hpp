//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_RACEGUI_H
#define HEADER_RACEGUI_H

#include <string>
#include <vector>

#include "base_gui.hpp"
#include "kart.hpp"
#include "material.hpp"
#include "player_kart.hpp"
#include "player.hpp"
#include "race_manager.hpp"
#include "modes/world.hpp"

class InputMap;
class RaceSetup;

/**
  * Used to display the list of karts and their times or
  * whatever other info is relevant to the current mode.
  */
struct KartIconDisplayInfo
{
    std::string time;
    float r, g, b;
    std::string special_title;
    /** Current lap of this kart, or -1 if irrelevant
      */
    int lap;
};

class RaceGUI: public BaseGUI
{

    class TimedMessage
    {
     public:
        std::string m_message;            // message to display
        float       m_remaining_time;     // time remaining before removing this message from screen
        int         m_red,m_blue,m_green; // colour
        int         m_font_size;          // size
        Kart        *m_kart;
        // std::vector needs standard copy-ctor and std-assignment op.
        // let compiler create defaults .. they'll do the job, no
        // deep copies here ..
        TimedMessage(const char *message, 
                     Kart *kart, float time, int size, 
                     int red, int green, int blue)
        {
            m_message    = message; 
            m_font_size  = size;
            m_kart       = kart;
            if( time < 0.0f ) m_remaining_time = -1.0f;
            else
            {
                m_remaining_time = time;
            }
            m_red=red; m_blue=blue; m_green=green; 
        }
        // in follow leader the clock counts backwards
        bool done(const float dt)
        {
            m_remaining_time -= dt;
            return m_remaining_time < 0;
        }
    };
public:

    RaceGUI();
    ~RaceGUI();
    void update(float dt);
    void select() {}
    void handle(GameAction, int);
    void handleKartAction(KartAction ka, int value);
    void addMessage(const char *message, Kart *kart, float time, int fonst_size,
                    int red=255, int green=0, int blue=255);

private:
    ulClock     m_fps_timer;
    int         m_fps_counter;
    char        m_fps_string[10];
    const char* m_pos_string [11];
    Material*   m_steering_wheel_icon;
    Material*   m_speed_back_icon;
    Material* m_speed_fore_icon;
    typedef   std::vector<TimedMessage> AllMessageType;
    AllMessageType m_messages;

    /* Display informat on screen */
    void drawStatusText        (const float dt);
    void drawEnergyMeter       (Kart *player_kart,
                                int   offset_x, int   offset_y,
                                float ratio_x,  float ratio_y  );
    void drawPowerupIcons  (Kart* player_kart,
                                int   offset_x, int   offset_y,
                                float ratio_x,  float ratio_y  );
    void drawAllMessages       (Kart* player_kart,
                                int   offset_x, int   offset_y,
                                float ratio_x,  float ratio_y  );
    void drawPlayerIcons       (const KartIconDisplayInfo* info);
    void oldDrawPlayerIcons    ();
    void drawMap               ();
    void drawTimer             ();
    void drawFPS               ();
    void drawMusicDescription  ();
    void cleanupMessages       (const float dt);

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
    void drawLap                  (const KartIconDisplayInfo* info, Kart* kart, int offset_x,
                                   int offset_y, float ratio_x, float ratio_y  );
};

#endif
