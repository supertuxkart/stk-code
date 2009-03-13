//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_FONT_H
#define HEADER_FONT_H

#include <string>


class Font
{
public:
    //CENTER_OF_SCREEN has to be bigger or smaller than Widget::MAX_SCROLL
    const static int CENTER_OF_SCREEN=-1000001;
    enum FontSize      {SMALL=18,   MEDIUM=24,   LARGE=30    };
    Font(const char* fontname);
    Font(const std::string &fontname) { Font(fontname.c_str()); }
    ~Font();
    void getBBox(const std::string &text, int size, bool italic,
                 float *left, float *right, float *bot, float *top);
    void getBBoxMultiLine(const std::string &text, int size, bool italic,
                          float *left, float *right, float *bot, float *top);

    // The actual main function which does everything
    // ----------------------------------------------
    void Print(      const char *text, int size,
                     int x, int y,
                     const float* color = NULL,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1,
                     bool doShadow=false);
    void Print(      std::string const &text, int size,
                     int x, int y,
                     const float* color = NULL,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1,
                     bool doShadow=false)
    {
        Print(text.c_str(), size, x, y,
              color, scale_x, scale_y, left, right, top, bottom,
              doShadow);
    }

    void PrintShadow(const char *text, int size,
                     int x, int y,
                     const float* color = NULL,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1)
    {
                     Print(text, size, x, y,
                           color, scale_x, scale_y,
                           left, right, top, bottom, true);
    }
    void PrintBold(  std::string const &text, int size,
                     int x, int y,
                     const float* color = NULL,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1);
};

int init_fonts();
int delete_fonts();

extern Font* font_gui;
extern Font* font_race;

#endif
