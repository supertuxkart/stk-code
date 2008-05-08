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

#include "file_manager.hpp"
#include "user_config.hpp"
#include "gui/font.hpp"

Font* font_gui;
Font* font_race;

int init_fonts()
{
    font_gui = new Font("DomesticMannersLatin1.txf");
    font_race = new Font("DomesticMannersLatin1.txf");
    return ( font_gui && font_race );
}   // init_fonts

// =============================================================================
int delete_fonts()
{
    delete font_gui;
    delete font_race;
    return 0;
}   // delete_fonts

// =============================================================================
Font::Font(const char *fontname)
{
    m_fnt      = new fntTexFont(file_manager->getFontFile(fontname).c_str(),
                                GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_text_out = new fntRenderer();
    m_text_out->setFont(m_fnt);
}   // Font

// -----------------------------------------------------------------------------
Font::~Font()
{
    delete m_text_out;
    delete m_fnt;
}   // ~Font

// -----------------------------------------------------------------------------

void Font::Print(const char *text, int size, 
                 int x, int y,
                 const GLfloat* color,
                 float scale_x, float scale_y,
                 int left, int right, int top, int bottom, bool doShadow)
{
    
    // Only scale for lower resolution
    float fontScaling = user_config->m_width<800 ? ((float)user_config->m_width/800.0f) 
                                                 : 1.0f;
    int   sz          = (int)(size*std::max(scale_x,scale_y)*fontScaling);

    float l,r,t,b;
    m_fnt->getBBox(text, (float)sz, 0, &l, &r, &b, &t);
    const int W = (int)((r-l+0.99));
    const int H = (int)((t-b+0.99));

    if(x==CENTER_OF_SCREEN)
    {
        if(left ==-1) left  = 0;
        if(right==-1) right = user_config->m_width-1;
        int width = right-left+1;
        x         = (width - W)/2 + left;
    }

    if(y==CENTER_OF_SCREEN)
    {
        if(top    == -1) top    = user_config->m_height-1;
        if(bottom == -1) bottom = 0;
        int height = top-bottom+1;
        y = (height - H)/2 + bottom;
    }

    m_text_out->begin();
    m_text_out->setPointSize((float)sz);
    if(doShadow)
    {
        m_text_out->start2f((GLfloat)x-2, (GLfloat)y-2);
        glColor4ub(0, 0, 0, 100);
        m_text_out->puts(text);
    }
    m_text_out->start2f((GLfloat)x, (GLfloat)y);

    if( color == NULL )
    {
        glColor4f(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        glColor4fv(color);
    }
    m_text_out->puts(text);
    m_text_out->end();

}   // Print
// -----------------------------------------------------------------------------

void Font::PrintBold(const std::string &text, int size, int x, int y,
                     const GLfloat* color, float scale_x, float scale_y,
                     int left, int right, int top, int bottom            )
{
    // Only scale for lower resolution
    float fontScaling = user_config->m_width<800 ? ((float)user_config->m_width/800.0f) 
                                                 : 1.0f;
    int   sz          = (int)(size*std::max(scale_x,scale_y)*fontScaling);

    float l,r,t,b;
    m_fnt->getBBox(text.c_str(), (float)sz, 0, &l, &r, &b, &t);
    const int W = (int)((r-l+0.99));
    const int H = (int)((t-b+0.99));

    if(x==CENTER_OF_SCREEN)
    {
        if(left ==-1) left  = 0;
        if(right==-1) right = user_config->m_width-1;
        int width = right-left+1;
        x         = (width - W)/2 + left;
    }

    if(y==CENTER_OF_SCREEN)
    {
        if(top    == -1) top    = user_config->m_height-1;
        if(bottom == -1) bottom = 0;
        int height = top-bottom+1;
        y = (height - H)/2 + bottom;
    }


    m_text_out->begin();
    m_text_out->setPointSize((float)sz);
    if( color == NULL )
    {
        glColor4f(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        glColor4fv(color);
    }
    for(float r=-1; r<=0; r+=0.5)
    {
        m_text_out->start2f((GLfloat)x-r, (GLfloat)y-r);
        m_text_out->puts(text.c_str());
    }
    m_text_out->end();

}   // PrintBold
