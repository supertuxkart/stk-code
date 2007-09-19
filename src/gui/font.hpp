//  $Id: font.hpp 907 2007-02-04 01:38:54Z coz $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_FONT_H
#define HEADER_FONT_H

#include <string>

#include <plib/fnt.h>

class Font
{
private:
    fntTexFont  *m_fnt;
    fntRenderer *m_text_out;

public:
    // Align right and top are not supported yet
    enum FontAlignType {ALIGN_LEFT, ALIGN_CENTER, ALIGN_BOTTOM};
    const static int CENTER_OF_SCREEN=-1;
    enum FontSize      {SMALL=18,   MEDIUM=24,   LARGE=30    };
    Font(const char* fontname);
    Font(const std::string &fontname) { Font(fontname.c_str()); }
    ~Font();
    void getBBox(const std::string &text, int size, bool italic,
                 float *left, float *right, float *bot, float *top)
    {
        m_fnt->getBBox(text.c_str(), size, italic, left, right, bot, top);
    }

    // The actual main function which does everything
    // ----------------------------------------------
    void Print(      const char *text, int size, 
                     FontAlignType fontalign_x, int x,
                     FontAlignType fontalign_y, int y,
                     int red=255, int green=255, int blue=255,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1,
                     bool doShadow=false);
    void Print(      std::string const &text, int size, 
                     FontAlignType fontalign_x, int x,
                     FontAlignType fontalign_y, int y,
                     int red=255, int green=255, int blue=255,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1,
                     bool doShadow=false)
    {
        Print(text.c_str(), size, fontalign_x, x, fontalign_y, y,
              red, green, blue, scale_x, scale_y, left, right, top, bottom,
              doShadow);
    }

    // Convenience functions to reduce the number of parameters
    // --------------------------------------------------------
    void Print(      const std::string &text, int size, int x, int y,
                     int red=255, int green=255, int blue=255,
                     int left=-1, int right=-1, int top=-1, int bottom=-1)
    {
                     Print(text,  size, ALIGN_LEFT, x, ALIGN_BOTTOM, y,
                           red, green, blue, 1.0f, 1.0f,
                           left, right, top, bottom);
    }

    void PrintShadow(const char *text, int size,
                     FontAlignType fontalign_x, int x,
                     FontAlignType fontalign_y, int y,
                     int red=255, int green=255, int blue=255,
                     float scale_x=1.0f, float scale_y=1.0f,
                     int left=-1, int right=-1, int top=-1, int bottom=-1)
    {
                     Print(text,  size, fontalign_x, x, fontalign_y, y,
                           red, green, blue, scale_x, scale_y,
                           left, right, top, bottom, true);
    }
    void PrintShadow(const char *text, int size, int x, int y,
                     int red=255, int green=255, int blue=255,
                     int left=-1, int right=-1, int top=-1, int bottom=-1)
    {
                     Print(text, size, ALIGN_LEFT, x, ALIGN_BOTTOM, y,
                           red, green, blue, 1.0f, 1.0f, 
                           left, right, top, bottom, true);
    }
};

int init_fonts();
int delete_fonts();

extern Font* font_gui;
extern Font* font_race;

#endif
