//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  This code originally from Neverball copyright (C) 2003 Robert Kooima
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

#include <SDL/SDL.h>

#include "constants.hpp"
#include "widget_set.hpp"
#include "loader.hpp"
#include "config.hpp"
#include "sound_manager.hpp"

WidgetSet *widgetSet;

WidgetSet::WidgetSet()
        : m_active(0), m_pause_id(0), m_paused(0)
{
    const int width   = config->m_width;
    const int height   = config->m_height;
    const int S   = (height < width) ? height : width ;
    m_radius  = S/60;
    m_fnt     = new fntTexFont(loader->getPath("fonts/AvantGarde-Demi.txf").c_str(),
                             GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_text_out = new fntRenderer();
    m_text_out->setFont(m_fnt);

    m_fnt_race = new fntTexFont(loader->getPath("fonts/DomesticManners.txf").c_str(),
                             GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_text_out_race = new fntRenderer();
    m_text_out_race->setFont(m_fnt_race);

    /* Initialize font rendering. */
    memset(m_widgets, 0, sizeof (Widget) * MAX_WIDGETS);

}

//-----------------------------------------------------------------------------
WidgetSet::~WidgetSet()
{
    /* Release any remaining widget texture and display list indices. */

    for (int id = 1; id < MAX_WIDGETS; id++)
    {
        if (glIsList(m_widgets[id].rect_obj))
            glDeleteLists(m_widgets[id].rect_obj, 1);

        m_widgets[id].type     = GUI_FREE;
        m_widgets[id].text_img = 0;
        m_widgets[id].rect_obj = 0;
        m_widgets[id].cdr      = 0;
        m_widgets[id].car      = 0;
    }

}

//-----------------------------------------------------------------------------
void WidgetSet::reInit()
{
    if(m_fnt    ) delete m_fnt;
    if(m_text_out) delete m_text_out;
    m_fnt     = new fntTexFont(loader->getPath("fonts/AvantGarde-Demi.txf").c_str(),
                             GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_text_out = new fntRenderer();
    m_text_out->setFont(m_fnt);
    if(m_text_out_race) delete m_text_out_race;
    m_fnt_race = new fntTexFont(loader->getPath("fonts/DomesticManners.txf").c_str(),
                             GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_text_out_race = new fntRenderer();
    m_text_out_race->setFont(m_fnt_race);

}   // reInit

//-----------------------------------------------------------------------------
int WidgetSet::hot(int id)
{
    return (m_widgets[id].type & GUI_STATE);
}


/** Initialize a display list containing a rounded-corner rectangle (x,
 *  y, w, h).  Generate texture coordinates to properly apply a texture
 *  map to the rectangle as though the corners were not rounded.
 */
GLuint WidgetSet::rect(int x, int y, int w, int h, int f, int r)
{
    GLuint list = glGenLists(1);

    const int N = 8;
    int i;

    glNewList(list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            /* Left side... */

            for (i = 0; i <= N; i++)
            {
                float a = 0.5f * M_PI * (float) i / (float) N;
                float s = r * sin(a);
                float c = r * cos(a);

                float X  = x     + r - c;
                float Ya = y + h + ((f & GUI_NW) ? (s - r) : 0);
                float Yb = y     + ((f & GUI_SW) ? (r - s) : 0);
                glTexCoord2f((X - x) / w, 1 - (Ya - y) / h);
                glVertex2f(X, Ya);

                glTexCoord2f((X - x) / w, 1 - (Yb - y) / h);
                glVertex2f(X, Yb);
            }

            /* ... Right side. */

            for (i = 0; i <= N; i++)
            {
                float a = 0.5f * M_PI * (float) i / (float) N;
                float s = r * sin(a);
                float c = r * cos(a);

                float X  = x + w - r + s;
                float Ya = y + h + ((f & GUI_NE) ? (c - r) : 0);
                float Yb = y     + ((f & GUI_SE) ? (r - c) : 0);

                glTexCoord2f((X - x) / w, 1 - (Ya - y) / h);
                glVertex2f(X, Ya);

                glTexCoord2f((X - x) / w, 1 - (Yb - y) / h);
                glVertex2f(X, Yb);
            }
        }
        glEnd();
    }
    glEndList();

    return list;
}

//-----------------------------------------------------------------------------
int WidgetSet::add_widget(int pd, int type)
{
    int id;

    /* Find an unused entry in the widget table. */

    for (id = 1; id < MAX_WIDGETS; id++)
    {
        if (m_widgets[id].type == GUI_FREE)
        {
            /* Set the type and default properties. */

            m_widgets[id].type       = type;
            m_widgets[id].token      = 0;
            m_widgets[id].value      = 0;
            m_widgets[id].size       = 0;
            m_widgets[id].rect       = GUI_NW | GUI_SW | GUI_NE | GUI_SE;
            m_widgets[id].w          = 0;
            m_widgets[id].h          = 0;
            m_widgets[id].text_img   = 0;
            m_widgets[id].rect_obj   = 0;
            m_widgets[id].color0     = gui_wht;
            m_widgets[id].color1     = gui_wht;
            m_widgets[id].scale      = 1.0f;
            m_widgets[id].count_text = (char*)NULL;

            /* Insert the new widget into the parents's widget list. */

            if (pd)
            {
                m_widgets[id].car = 0;
                m_widgets[id].cdr = m_widgets[pd].car;
                m_widgets[pd].car = id;
            }
            else
            {
                m_widgets[id].car = 0;
                m_widgets[id].cdr = 0;
            }

            return id;
        }
    }   // for i

    fprintf(stderr, "Out of widget IDs\n");

    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::harray(int pd) { return add_widget(pd, GUI_HARRAY); }
//-----------------------------------------------------------------------------
int WidgetSet::varray(int pd) { return add_widget(pd, GUI_VARRAY); }
//-----------------------------------------------------------------------------
int WidgetSet::hstack(int pd) { return add_widget(pd, GUI_HSTACK); }
//-----------------------------------------------------------------------------
int WidgetSet::vstack(int pd) { return add_widget(pd, GUI_VSTACK); }
//-----------------------------------------------------------------------------
int WidgetSet::filler(int pd) { return add_widget(pd, GUI_FILLER); }

//-----------------------------------------------------------------------------
void WidgetSet::set_label(int id, const char *text)
{
    float l,r,b,t;
    m_fnt->getBBox(text, m_widgets[id].size, 0.0f, &l, &r, &b, &t);
    m_widgets[id].yOffset    = (int)b;
    m_widgets[id].text_width = (int)(r-l+0.99);
    m_widgets[id]._text      = text;
    // There is a potential bug here: if the label being set is
    // larger than the current width, the container (parent) does
    // not get wider ... the layout will be broken. Unfortunately,
    // that's somewhat difficult to fix, since layout will in turn
    // add the radius to width of button (see button_up), ...
    // So for now we only print a warning:
    if(m_widgets[id].text_width > m_widgets[id].w)
    {
        fprintf(stderr,
                "set_label increased width of parent container, layout will be invalid\n");
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::set_count(int id, int value)
{
    m_widgets[id].value = value;
    sprintf(m_widgets[id].count_text,"%d",value);
    float l,r,b,t;
    m_fnt->getBBox(m_widgets[id].count_text, m_widgets[id].size, 0, &l, &r, &b, &t);
    m_widgets[id].yOffset    = (int)(b);
    m_widgets[id].w          = (int)(r-l+0.99);
    m_widgets[id].h          = (int)(t-b+0.99);
}   // set_count

//-----------------------------------------------------------------------------
void WidgetSet::set_clock(int id, int value)
{
    m_widgets[id].value = value;
}

//-----------------------------------------------------------------------------
void WidgetSet::set_multi(int id, const char *text)
{
    const char *p;

    char s[8][MAX_STR];
    int  i, j, jd;

    size_t n = 0;

    /* Copy each delimited string to a line buffer. */

    for (p = text, j = 0; *p && j < 8; j++)
    {
        strncpy(s[j], p, (n = strcspn(p, "\\")));
        s[j][n] = 0;

        if (*(p += n) == '\\') p++;
    }

    /* Set the label value for each line. */

    for (i = j - 1, jd = m_widgets[id].car; i >= 0 && jd; i--, jd = m_widgets[jd].cdr)
        set_label(jd, s[i]);
}

//-----------------------------------------------------------------------------
int WidgetSet::image(int pd, int textureId, int w, int h)
{
    int id;

    if ((id = add_widget(pd, GUI_IMAGE)))
    {
        m_widgets[id].text_img = textureId;
        m_widgets[id].w     = w;
        m_widgets[id].h     = h;
    }
    return id;
}

//-----------------------------------------------------------------------------
int WidgetSet::start(int pd, const char *text, int size, int token, int value)
{
    int id;

    if ((id = state(pd, text, size, token, value)))
        m_active = id;

    return id;
}

//-----------------------------------------------------------------------------
int WidgetSet::state(int pd, const char *text, int size, int token, int value)
{
    int id;

    if ((id = add_widget(pd, GUI_STATE)))
    {
        m_widgets[id]._text = text;
        float l,r,b,t;
        m_fnt->getBBox(text, size, 0, &l, &r, &b, &t);
        // Apparently the font system allows charater to be
        // under the y=0 line (e.g the character g, p, y)
        // Since the text will not be centered because of this,
        // the distance to the baseline y=0 is saved and during
        // output added to the y location.
        m_widgets[id].w          = (int)(r-l+0.99);
        m_widgets[id].text_width = m_widgets[id].w;
        m_widgets[id].h          = (int)(t-b+0.99);
        m_widgets[id].yOffset    = (int)b - m_widgets[id].h/4 ;
        m_widgets[id].size       = size;
        m_widgets[id].token      = token;
        m_widgets[id].value      = value;
    }
    return id;
}

//-----------------------------------------------------------------------------
int WidgetSet::label(int pd, const char *text, int size, int rect, const float *c0,
                     const float *c1)
{
    int id;

    if ((id = add_widget(pd, GUI_LABEL)))
    {
        m_widgets[id]._text = text;
        float l,r,b,t;
        m_fnt->getBBox(text, size, 0, &l, &r, &b, &t);
        m_widgets[id].yOffset    = (int)(b);
        m_widgets[id].w          = (int)(r-l+0.99);
        m_widgets[id].h          = (int)(t-b+0.99);
        m_widgets[id].text_width = m_widgets[id].w;
        m_widgets[id].size       = size;
        m_widgets[id].color0     = c0 ? c0 : gui_yel;
        m_widgets[id].color1     = c1 ? c1 : gui_red;
        m_widgets[id].rect       = rect;
    }
    return id;
}

//-----------------------------------------------------------------------------
int WidgetSet::count(int pd, int value, int size, int rect)
{
    int  id;

    if ((id = add_widget(pd, GUI_COUNT)))
    {
        m_widgets[id].value  = value;
        m_widgets[id].size   = size;
        m_widgets[id].color0 = gui_yel;
        m_widgets[id].color1 = gui_red;
        m_widgets[id].rect   = rect;
        m_widgets[id].count_text  = new char[20];
        sprintf(m_widgets[id].count_text,"%d",value);
        float l,r,b,t;
        m_fnt->getBBox(m_widgets[id].count_text, size, 0, &l, &r, &b, &t);
        m_widgets[id].yOffset    = (int)(b);
        m_widgets[id].w          = (int)(r-l+0.99);
        m_widgets[id].h          = (int)(t-b+0.99);
    }
    return id;
}   // count

//-----------------------------------------------------------------------------
int WidgetSet::clock(int pd, int value, int size, int rect)
{
    int id;

    if ((id = add_widget(pd, GUI_CLOCK)))
    {
        printf("clock: FIXME\n");
        //m_widgets[id].w      = digit_w[size][0] * 6;
        //m_widgets[id].h      = digit_h[size][0];
        m_widgets[id].value  = value;
        m_widgets[id].size   = size;
        m_widgets[id].color0 = gui_yel;
        m_widgets[id].color1 = gui_red;
        m_widgets[id].rect   = rect;
    }
    return id;
}

//-----------------------------------------------------------------------------
int WidgetSet::space(int pd)
{
    int id;

    if ((id = add_widget(pd, GUI_SPACE)))
    {
        m_widgets[id].w = 0;
        m_widgets[id].h = 0;
    }
    return id;
}

//-----------------------------------------------------------------------------
void WidgetSet::drawText (const char *text, int sz, int x, int y,
                          int red, int green, int blue,
                          float scale_x, float scale_y )
{
    float l,r,t,b, fontScaling;
    // Only scale for lower resolution
    fontScaling = config->m_width<800 ? ((float)config->m_width/800.0f) : 1.0f;
    fontScaling = (float)config->m_width/800.0f;
    sz = (int)(sz*std::max(scale_x,scale_y)*fontScaling);
    m_fnt->getBBox(text, sz, 0, &l, &r, &b, &t);
    const int W = (int)((r-l+0.99)*scale_x);
    const int H = (int)((t-b+0.99)*scale_y);
    if(x == SCREEN_CENTERED_TEXT)
    {
        x = (config->m_width - W) / 2;
    }
    if(y == SCREEN_CENTERED_TEXT)
    {
        y = (config->m_height - H) / 2;
    }

    m_text_out->begin();
    m_text_out->setPointSize(sz);
    m_text_out->start2f((GLfloat)x, (GLfloat)y);
    glColor3ub(red, green, blue);
    m_text_out->puts(text);
    m_text_out->end();
}

//-----------------------------------------------------------------------------
void WidgetSet::drawDropShadowText (const char *text, int sz, int x, int y,
                                    int red, int green, int blue,
                                    float scale_x, float scale_y )
{
    drawText ( text, sz, x, y, 0, 0, 0, scale_x, scale_y ) ;
    drawText ( text, sz, x+2, y+2, red, green, blue, scale_x, scale_y ) ;
}

//-----------------------------------------------------------------------------
void WidgetSet::drawTextRace (const char *text, int sz, int x, int y,
                              int red, int green, int blue,
                              float scale_x, float scale_y )
{
    float l,r,t,b, fontScaling;
    // Only scale for lower resolution
    fontScaling = config->m_width<800 ? ((float)config->m_width/800.0f) : 1.0f;
    fontScaling = (float)config->m_width/800.0f;
    sz = (int)(sz*std::max(scale_x,scale_y)*fontScaling);
    m_fnt_race->getBBox(text, sz, 0, &l, &r, &b, &t);
    const int W = (int)((r-l+0.99)*scale_x);
    const int H = (int)((t-b+0.99)*scale_y);
    if(x == SCREEN_CENTERED_TEXT)
    {
        x = (config->m_width - W) / 2;
    }
    if(y == SCREEN_CENTERED_TEXT)
    {
        y = (config->m_height - H) / 2;
    }

    m_text_out_race->begin();
    m_text_out_race->setPointSize(sz);
    m_text_out_race->start2f((GLfloat)x, (GLfloat)y);
    glColor3ub(red, green, blue);
    m_text_out_race->puts(text);
    m_text_out_race->end();
}

//-----------------------------------------------------------------------------
void WidgetSet::drawDropShadowTextRace (const char *text, int sz, int x, int y,
                                        int red, int green, int blue,
                                        float scale_x, float scale_y )
{
    drawTextRace ( text, sz, x, y, 0, 0, 0, scale_x, scale_y ) ;
    drawTextRace ( text, sz, x+2, y+2, red, green, blue, scale_x, scale_y ) ;
}

//-----------------------------------------------------------------------------
int WidgetSet::pause(int pd)
{
    int id;

    if ((id = add_widget(pd, GUI_PAUSE)))
    {
        m_widgets[id].value  = 0;
        m_widgets[id].rect   = GUI_ALL;
    }
    return id;
}

/** Create  a multi-line  text box  using a  vertical array  of labels.
 *  Parse the  text for '\'  characters and treat them  as line-breaks.
 *  Preserve the rect specifation across the entire array.
 */
int WidgetSet::multi(int pd, const char *text, int size, int rect, const float *c0,
                     const float *c1)
{
    int id = 0;

    if (text && (id = varray(pd)))
    {
        const char *p;
#define MAX_NUMBER_OF_LINES 20
        // Important; this s must be static, otherwise the strings will be
        // lost when returning --> garbage will be displayed.
        static char s[MAX_NUMBER_OF_LINES][MAX_STR];
        int  r[MAX_NUMBER_OF_LINES];
        int  i, j;

        size_t n = 0;

        /* Copy each delimited string to a line buffer. */

        for (p = text, j = 0; *p && j < MAX_NUMBER_OF_LINES; j++)
        {
            strncpy(s[j], p, (n = strcspn(p, "\n")));
            s[j][n] = 0;
            r[j]    = 0;

            if (*(p += n) == '\n') p++;
        }

        /* Set the curves for the first and last lines. */

        if (j > 0)
        {
            r[0]     |= rect & (GUI_NW | GUI_NE);
            r[j - 1] |= rect & (GUI_SW | GUI_SE);
        }

        /* Create a label widget for each line. */

        for (i = 0; i < j; i++)
            label(id, s[i], size, r[i], c0, c1);
    }
    return id;
}

/** The bottom-up pass determines the area of all widgets.  The minimum
 *  width  and height of  a leaf  widget is  given by  the size  of its
 *  contents.   Array  and  stack   widths  and  heights  are  computed
 *  recursively from these.
 */
void WidgetSet::harray_up(int id)
{
    int jd, c = 0;

    /* Find the widest child width and the highest child height. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        widget_up(jd);

        if (m_widgets[id].h < m_widgets[jd].h)
            m_widgets[id].h = m_widgets[jd].h;
        if (m_widgets[id].w < m_widgets[jd].w)
            m_widgets[id].w = m_widgets[jd].w;
        c++;
    }

    /* Total width is the widest child width times the child count. */

    m_widgets[id].w *= c;
}

//-----------------------------------------------------------------------------
void WidgetSet::varray_up(int id)
{
    int jd, c = 0;

    /* Find the widest child width and the highest child height. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        widget_up(jd);

        if (m_widgets[id].h < m_widgets[jd].h)
            m_widgets[id].h = m_widgets[jd].h;
        if (m_widgets[id].w < m_widgets[jd].w)
            m_widgets[id].w = m_widgets[jd].w;

        c++;
    }

    /* Total height is the highest child height times the child count. */

    m_widgets[id].h *= c;
}

//-----------------------------------------------------------------------------
void WidgetSet::hstack_up(int id)
{
    int jd;

    /* Find the highest child height.  Sum the child widths. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        widget_up(jd);

        if (m_widgets[id].h < m_widgets[jd].h)
            m_widgets[id].h = m_widgets[jd].h;

        m_widgets[id].w += m_widgets[jd].w;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::vstack_up(int id)
{
    int jd;

    /* Find the widest child width.  Sum the child heights. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        widget_up(jd);

        if (m_widgets[id].w < m_widgets[jd].w)
            m_widgets[id].w = m_widgets[jd].w;

        m_widgets[id].h += m_widgets[jd].h;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::paused_up(int id)
{
    /* Store width and height for later use in text rendering. */

    m_widgets[id].x = m_widgets[id].w;
    m_widgets[id].y = m_widgets[id].h;

    /* The pause widget fills the screen. */

    m_widgets[id].w = config->m_width;
    m_widgets[id].h = config->m_height;
}

//-----------------------------------------------------------------------------
void WidgetSet::button_up(int id)
{
    /* Store width and height for later use in text rendering. */

    m_widgets[id].x = m_widgets[id].w;
    m_widgets[id].y = m_widgets[id].h;

    if (m_widgets[id].w < m_widgets[id].h && m_widgets[id].w > 0)
        m_widgets[id].w = m_widgets[id].h;


    /* Padded text elements look a little nicer. */

    if (m_widgets[id].w < config->m_width)
        m_widgets[id].w += m_radius;
    if (m_widgets[id].h < config->m_height)
        m_widgets[id].h += m_radius;
}

//-----------------------------------------------------------------------------
void WidgetSet::widget_up(int id)
{
    if (id)
        switch (m_widgets[id].type & GUI_TYPE)
        {
        case GUI_HARRAY: harray_up(id); break;
        case GUI_VARRAY: varray_up(id); break;
        case GUI_HSTACK: hstack_up(id); break;
        case GUI_VSTACK: vstack_up(id); break;
        case GUI_PAUSE:  paused_up(id); break;
        default:         button_up(id); break;
        }
}

/** The  top-down layout  pass distributes  available area  as computed
 *  during the bottom-up pass.  Widgets  use their area and position to
 *  initialize rendering state.
 */
void WidgetSet::harray_dn(int id, int x, int y, int w, int h)
{
    int jd, i = 0, c = 0;

    m_widgets[id].x = x;
    m_widgets[id].y = y;
    m_widgets[id].w = w;
    m_widgets[id].h = h;

    /* Count children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        c += 1;

    /* Distribute horizontal space evenly to all children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr, i++)
    {
        int x0 = x +  i      * w / c;
        int x1 = x + (i + 1) * w / c;

        widget_dn(jd, x0, y, x1 - x0, h);
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::varray_dn(int id, int x, int y, int w, int h)
{
    int jd, i = 0, c = 0;

    m_widgets[id].x = x;
    m_widgets[id].y = y;
    m_widgets[id].w = w;
    m_widgets[id].h = h;

    /* Count children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        c += 1;

    /* Distribute vertical space evenly to all children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr, i++)
    {
        int y0 = y +  i      * h / c;
        int y1 = y + (i + 1) * h / c;

        widget_dn(jd, x, y0, w, y1 - y0);
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::hstack_dn(int id, int x, int y, int w, int h)
{
    int jd, jx = x, jw = 0, c = 0;

    m_widgets[id].x = x;
    m_widgets[id].y = y;
    m_widgets[id].w = w;
    m_widgets[id].h = h;

    /* Measure the total width requested by non-filler children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        if ((m_widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            c += 1;
        else
            jw += m_widgets[jd].w;

    /* Give non-filler children their requested space.   */
    /* Distribute the rest evenly among filler children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        if ((m_widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            widget_dn(jd, jx, y, (w - jw) / c, h);
        else
            widget_dn(jd, jx, y, m_widgets[jd].w, h);

        jx += m_widgets[jd].w;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::vstack_dn(int id, int x, int y, int w, int h)
{
    int jd, jy = y, jh = 0, c = 0;

    m_widgets[id].x = x;
    m_widgets[id].y = y;
    m_widgets[id].w = w;
    m_widgets[id].h = h;

    /* Measure the total height requested by non-filler children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        if ((m_widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            c += 1;
        else
            jh += m_widgets[jd].h;

    /* Give non-filler children their requested space.   */
    /* Distribute the rest evenly among filler children. */

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        if ((m_widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            widget_dn(jd, x, jy, w, (h - jh) / c);
        else
            widget_dn(jd, x, jy, w, m_widgets[jd].h);

        jy += m_widgets[jd].h;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::filler_dn(int id, int x, int y, int w, int h)
{
    /* Filler expands to whatever size it is given. */

    m_widgets[id].x = x;
    m_widgets[id].y = y;
    m_widgets[id].w = w;
    m_widgets[id].h = h;
}

//-----------------------------------------------------------------------------
void WidgetSet::button_dn(int id, int x, int y, int w, int h)
{
    /* Recall stored width and height for text rendering. */

    int R = m_widgets[id].rect;
    int r = ((m_widgets[id].type & GUI_TYPE) == GUI_PAUSE ? m_radius * 4 : m_radius);

    m_widgets[id].x = x;
    m_widgets[id].y = y;
    m_widgets[id].w = w;
    m_widgets[id].h = h;
    /* Create display lists for the text area and rounded rectangle. */

    m_widgets[id].rect_obj = rect(-w / 2, -h / 2, w, h, R, r);
}

//-----------------------------------------------------------------------------
void WidgetSet::widget_dn(int id, int x, int y, int w, int h)
{
    if (id)
        switch (m_widgets[id].type & GUI_TYPE)
        {
        case GUI_HARRAY: harray_dn(id, x, y, w, h); break;
        case GUI_VARRAY: varray_dn(id, x, y, w, h); break;
        case GUI_HSTACK: hstack_dn(id, x, y, w, h); break;
        case GUI_VSTACK: vstack_dn(id, x, y, w, h); break;
        case GUI_FILLER: filler_dn(id, x, y, w, h); break;
        case GUI_SPACE:  filler_dn(id, x, y, w, h); break;
        default:         button_dn(id, x, y, w, h); break;
        }
}

/** During GUI layout, we make a bottom-up pass to determine total area
 *  requirements for  the widget  tree.  We position  this area  to the
 *  sides or center of the screen.  Finally, we make a top-down pass to
 *  distribute this area to each widget.
 */
void WidgetSet::layout(int id, int xd, int yd)
{
    int x, y;

    const int W = config->m_width;
    const int H = config->m_height;
    int w = W;
    int h = W;

    widget_up(id);

    w = m_widgets[id].w;
    h = m_widgets[id].h;
    if      (xd < 0) x = 0;
    else if (xd > 0) x = (W - w);
    else             x = (W - w) / 2;

    if      (yd < 0) y = 0;
    else if (yd > 0) y = (H - h);
    else             y = (H - h) / 2;

    widget_dn(id, x, y, w, h);

    /* Hilite the widget under the cursor, if any. */

    point(id, -1, -1);
}

//-----------------------------------------------------------------------------
int WidgetSet::search(int id, int x, int y)
{
    int jd, kd;
    assert(id < MAX_WIDGETS);

    /* Search the hierarchy for the widget containing the given point. */

    if (id && (m_widgets[id].x <= x && x < m_widgets[id].x + m_widgets[id].w &&
               m_widgets[id].y <= y && y < m_widgets[id].y + m_widgets[id].h))
    {
        if (hot(id))
            return id;

        for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
            if ((kd = search(jd, x, y)))
                return kd;
    }
    return 0;
}

/** Activate a widget, allowing it  to behave as a normal state widget.
 *  This may  be used  to create  image buttons, or  cause an  array of
 *  widgets to behave as a single state widget.
 */
int WidgetSet::activate_widget(int id, int token, int value)
{
    m_widgets[id].type |= GUI_STATE;
    m_widgets[id].token = token;
    m_widgets[id].value = value;

    return id;
}

//-----------------------------------------------------------------------------
int WidgetSet::delete_widget(int id)
{
    if (id)
    {
        /* Recursively delete all subwidgets. */

        delete_widget(m_widgets[id].cdr);
        delete_widget(m_widgets[id].car);

        /* Release any GL resources held by this widget. */

        if (glIsList(m_widgets[id].rect_obj))
            glDeleteLists(m_widgets[id].rect_obj, 1);

        /* Mark this widget unused. */
        if(m_widgets[id].type==GUI_COUNT)
        {
            delete m_widgets[id].count_text;
        }
        m_widgets[id].type     = GUI_FREE;
        m_widgets[id].text_img = 0;
        m_widgets[id].rect_obj = 0;
        m_widgets[id].cdr      = 0;
        m_widgets[id].car      = 0;
    }
    return 0;
}

//-----------------------------------------------------------------------------
void WidgetSet::paint_rect(int id, int st)
{
#ifdef SNIP
    static const GLfloat back[4][4] =
        {
            { 0.1f, 0.1f, 0.1f, 0.5f }
            ,             /* off and inactive    */
            { 0.3f, 0.3f, 0.3f, 0.5f },             /* off and   active    */
            { 0.7f, 0.3f, 0.0f, 0.5f },             /* on  and inactive    */
            { 1.0f, 0.7f, 0.3f, 0.5f },             /* on  and   active    */
        };
#endif
    static const GLfloat back[4][4] =
        {
            { 0.1f, 0.1f, 0.1f, 0.5f }
            ,             /* off and inactive    */
            { 0.5f, 0.5f, 0.5f, 0.8f },             /* off and   active    */
            { 1.0f, 0.7f, 0.3f, 0.5f },             /* on  and inactive    */
            { 1.0f, 0.7f, 0.3f, 0.8f },             /* on  and   active    */
        };

    int jd, i = 0;

    /* Use the widget status to determine the background color. */

    if (hot(id))
        i = st | (((m_widgets[id].value) ? 2 : 0) |
                  ((id == m_active)     ? 1 : 0));
    switch (m_widgets[id].type & GUI_TYPE)
    {
    case GUI_IMAGE:
    case GUI_SPACE:
    case GUI_FILLER:
        break;

    case GUI_HARRAY:
    case GUI_VARRAY:
    case GUI_HSTACK:
    case GUI_VSTACK:

        /* Recursively paint all subwidgets. */

        for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
            paint_rect(jd, i);

        break;

    default:

        /* Draw a leaf's background, colored by widget state. */
        glPushMatrix();
        {
            glTranslatef((GLfloat) (m_widgets[id].x + m_widgets[id].w / 2),
                         (GLfloat) (m_widgets[id].y + m_widgets[id].h / 2), 0.f);

            glColor4fv(back[i]);
            glCallList(m_widgets[id].rect_obj);
        }
        glPopMatrix();

        break;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::paint_array(int id)
{
    int jd;

    glPushMatrix();
    {
        GLfloat cx = m_widgets[id].x + m_widgets[id].w / 2.0f;
        GLfloat cy = m_widgets[id].y + m_widgets[id].h / 2.0f;
        GLfloat ck = m_widgets[id].scale;

        glTranslatef(+cx, +cy, 0.0f);
        glScalef(ck, ck, ck);
        glTranslatef(-cx, -cy, 0.0f);

        /* Recursively paint all subwidgets. */

        for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
            paint_text(jd);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
void WidgetSet::paint_image(int id)
{
    /* Draw the widget rect, textured using the image. */

    glPushMatrix();
    {
        glTranslatef((GLfloat) (m_widgets[id].x + m_widgets[id].w / 2),
                     (GLfloat) (m_widgets[id].y + m_widgets[id].h / 2), 0.f);
        /* For whatever reasons the icons are upside down,
           so we mirror them back to the right orientation */
        glScalef(m_widgets[id].scale,
                 -m_widgets[id].scale,
                 m_widgets[id].scale);

        glBindTexture(GL_TEXTURE_2D, m_widgets[id].text_img);
        glColor4fv(gui_wht);
        glCallList(m_widgets[id].rect_obj);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
void WidgetSet::paint_count(int id)
{
    drawText(m_widgets[id].count_text, m_widgets[id].size, m_widgets[id].x,m_widgets[id].y,
             255,255,255,1.0, 1.0);
    return;
}   // paint_count

//-----------------------------------------------------------------------------
void WidgetSet::paint_clock(int id)
{
    //int i  =   m_widgets[id].size;
    const int MT =  (m_widgets[id].value / 6000) / 10;
    //int mo =  (m_widgets[id].value / 6000) % 10;
    //int st = ((m_widgets[id].value % 6000) / 100) / 10;
    //int so = ((m_widgets[id].value % 6000) / 100) % 10;
    //int ht = ((m_widgets[id].value % 6000) % 100) / 10;
    //int ho = ((m_widgets[id].value % 6000) % 100) % 10;

    GLfloat dx_large=1.0f;// FIXME = (GLfloat) digit_w[i][0];
    GLfloat dx_small=0.1f;// = (GLfloat) digit_w[i][0] * 0.75f;

    printf("paint_clock: FIXME\n");
    glPushMatrix();
    {
        glColor4fv(gui_wht);

        /* Translate to the widget center, and apply the pulse scale. */

        glTranslatef((GLfloat) (m_widgets[id].x + m_widgets[id].w / 2),
                     (GLfloat) (m_widgets[id].y + m_widgets[id].h / 2), 0.f);

        glScalef(m_widgets[id].scale,
                 m_widgets[id].scale,
                 m_widgets[id].scale);

        /* Translate left by half the total width of the rendered value. */

        if (MT > 0)
            glTranslatef(-2.25f * dx_large, 0.0f, 0.0f);
        else
            glTranslatef(-1.75f * dx_large, 0.0f, 0.0f);

        /* Render the minutes counter. */

        if (MT > 0)
        {
            //glBindTexture(GL_TEXTURE_2D, digit_text[i][MT]);
            // glCallList(digit_list[i][MT]);
            // glTranslatef(dx_large, 0.0f, 0.0f);
        }

        //glBindTexture(GL_TEXTURE_2D, digit_text[i][mo]);
        //glCallList(digit_list[i][mo]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render the colon. */

        //glBindTexture(GL_TEXTURE_2D, digit_text[i][10]);
        //glCallList(digit_list[i][10]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render the seconds counter. */

        //glBindTexture(GL_TEXTURE_2D, digit_text[i][st]);
        //glCallList(digit_list[i][st]);
        glTranslatef(dx_large, 0.0f, 0.0f);

        //glBindTexture(GL_TEXTURE_2D, digit_text[i][so]);
        //glCallList(digit_list[i][so]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render hundredths counter half size. */

        glScalef(0.5f, 0.5f, 1.0f);

        //glBindTexture(GL_TEXTURE_2D, digit_text[i][ht]);
        //glCallList(digit_list[i][ht]);
        glTranslatef(dx_large, 0.0f, 0.0f);

        //glBindTexture(GL_TEXTURE_2D, digit_text[i][ho]);
        //glCallList(digit_list[i][ho]);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
void WidgetSet::paint_label(int id)
{
    /* Draw the widget text box, textured using the glyph. */

    glPushMatrix();
    {
        // These offsets are rather horrible, but at least they
        // seem to give the right visuals
        glTranslatef((GLfloat) (m_widgets[id].x + m_widgets[id].w+
                                (m_radius-m_widgets[id].text_width)/2),
                     (GLfloat) (m_widgets[id].y + (m_widgets[id].h-m_radius)/2), 0.f);

        glScalef(m_widgets[id].scale, m_widgets[id].scale, m_widgets[id].scale);
        m_text_out->begin();
        {
            m_text_out->setPointSize(m_widgets[id].size);
            m_text_out->start2f((GLfloat)-m_widgets[id].w/2 , (GLfloat)m_widgets[id].yOffset);
            m_text_out->puts(m_widgets[id]._text);
        }
        m_text_out->end();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
void WidgetSet::paint_text(int id)
{
    switch (m_widgets[id].type & GUI_TYPE)
    {
    case GUI_SPACE:  break;
    case GUI_FILLER: break;
    case GUI_HARRAY: paint_array(id); break;
    case GUI_VARRAY: paint_array(id); break;
    case GUI_HSTACK: paint_array(id); break;
    case GUI_VSTACK: paint_array(id); break;
    case GUI_IMAGE:  paint_image(id); break;
    case GUI_COUNT:  paint_count(id); break;
    case GUI_CLOCK:  paint_clock(id); break;
    default:         paint_label(id); break;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::paint(int id)
{
    if (id)
    {
        glPushAttrib(GL_LIGHTING_BIT     |
                     GL_COLOR_BUFFER_BIT |
                     GL_DEPTH_BUFFER_BIT);
        config_push_ortho();

        glEnable(GL_BLEND);
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE  ) ;

        glPushAttrib(GL_TEXTURE_BIT);
        {
            glDisable(GL_TEXTURE_2D);
            paint_rect(id, 0);
        }
        glPopAttrib();

        // Makes the font white ... not sure how to get
        if(m_widgets[id].color0)
        {
            glColor4fv(m_widgets[id].color0);
        }
        else
        {
#ifdef WIN32
            // This appears to be a visual c++ bug
            GLfloat dummy5[4]={ 1.f, 1.f, 1.f, .5f };
            glColor4fv(dummy5);
            GLfloat dummy1[4]={ 1.f, 1.f, 1.f, 1.f };
            glColor4fv(dummy1);
#else
            glColor4fv((GLfloat[4]){ 1.f, 1.f, 1.f, .5f }
                      );
            glColor4fv((GLfloat[4]){ 1.f, 1.f, 1.f, 1.f }
                      );
#endif

        }
        paint_text(id);

        config_pop_matrix();
        glPopAttrib();
    }
}

#if 0
//-----------------------------------------------------------------------------
void WidgetSet::blank()
{
    paint(m_pause_id);
}
#endif

//-----------------------------------------------------------------------------
void WidgetSet::dump(int id, int d)
{
    int jd, i;

    if (id)
    {
        char *type = "?";

        switch (m_widgets[id].type & GUI_TYPE)
        {
        case GUI_HARRAY: type = "harray"; break;
        case GUI_VARRAY: type = "varray"; break;
        case GUI_HSTACK: type = "hstack"; break;
        case GUI_VSTACK: type = "vstack"; break;
        case GUI_FILLER: type = "filler"; break;
        case GUI_IMAGE:  type = "image";  break;
        case GUI_LABEL:  type = "label";  break;
        case GUI_COUNT:  type = "count";  break;
        case GUI_CLOCK:  type = "clock";  break;
        }

        for (i = 0; i < d; i++)
            printf("    ");

        printf("%04d %s\n", id, type);

        for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
            dump(jd, d + 1);
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::pulse(int id, float k)
{
    if (id) m_widgets[id].scale = k;
}

//-----------------------------------------------------------------------------
void WidgetSet::timer(int id, float dt)
{
    int jd;

    if (id)
    {
        for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
            timer(jd, dt);

        if (m_widgets[id].scale - 1.0f < dt)
            m_widgets[id].scale = 1.0f;
        else
            m_widgets[id].scale -= dt;
    }
}

//-----------------------------------------------------------------------------
int WidgetSet::point(int id, int x, int y)
{
    static int x_cache = 0;
    static int y_cache = 0;

    int jd;

    /* Reuse the last coordinates if (x,y) == (-1,-1) */

    if (x < 0 && y < 0)
        return point(id, x_cache, y_cache);

    x_cache = x;
    y_cache = y;

    /* Short-circuit check the current active widget. */

    jd = search(m_active, x, y);

    /* If not still active, search the hierarchy for a new active widget. */

    if (jd == 0)
        jd = search(id, x, y);

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == m_active)
        return 0;
    else
        return m_active = jd;
}

//-----------------------------------------------------------------------------
int WidgetSet::click(void)
{
    return m_active;
}

//-----------------------------------------------------------------------------
int WidgetSet::token(int id) const
{
    return id ? m_widgets[id].token : 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::value(int id) const
{
    return id ? m_widgets[id].value : 0;
}

//-----------------------------------------------------------------------------
void WidgetSet::toggle(int id)
{
    m_widgets[id].value = m_widgets[id].value ? 0 : 1;
}

//-----------------------------------------------------------------------------
int WidgetSet::vert_test(int id, int jd)
{
    /* Determine whether widget id is in vertical contact with widget jd. */

    if (id && (m_widgets[id].type&(GUI_STATE|GUI_SPACE)) &&
        jd && (m_widgets[jd].type&(GUI_STATE|GUI_SPACE))    )
    {
        const int I0 = m_widgets[id].x;
        const int I1 = m_widgets[id].x + m_widgets[id].w;
        const int J0 = m_widgets[jd].x;
        const int J1 = m_widgets[jd].x + m_widgets[jd].w;

        /* Is widget id's top edge is in contact with jd's bottom edge? */

        if (m_widgets[id].y + m_widgets[id].h == m_widgets[jd].y)
        {
            /* Do widgets id and jd overlap horizontally? */

            if (J0 <= I0 && I0 <  J1) return 1;
            if (J0 <  I1 && I1 <= J1) return 1;
            if (I0 <= J0 && J0 <  I1) return 1;
            if (I0 <  J1 && J1 <= I1) return 1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::horz_test(int id, int jd)
{
    /* Determine whether widget id is in horizontal contact with widget jd. */

    if (id && hot(id) && jd && hot(jd))
    {
        const int I0 = m_widgets[id].y;
        const int I1 = m_widgets[id].y + m_widgets[id].h;
        const int J0 = m_widgets[jd].y;
        const int J1 = m_widgets[jd].y + m_widgets[jd].h;

        /* Is widget id's right edge in contact with jd's left edge? */

        if (m_widgets[id].x + m_widgets[id].w == m_widgets[jd].x)
        {
            /* Do m_widgets id and jd overlap vertically? */

            if (J0 <= I0 && I0 <  J1) return 1;
            if (J0 <  I1 && I1 <= J1) return 1;
            if (I0 <= J0 && J0 <  I1) return 1;
            if (I0 <  J1 && J1 <= I1) return 1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::stick_L(int id, int dd)
{
    int jd, kd;

    /* Find a widget to the left of widget dd. */

    if (horz_test(id, dd))
        return id;

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        if ((kd = stick_L(jd, dd)))
            return kd;

    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::stick_R(int id, int dd)
{
    int jd, kd;

    /* Find a widget to the right of widget dd. */

    if (horz_test(dd, id))
        return id;

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        if ((kd = stick_R(jd, dd)))
            return kd;

    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::stick_D(int id, int dd)
{
    int jd, kd;

    /* Find a widget below widget dd. */

    if (vert_test(id, dd))
        return id;

    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
        if ((kd = stick_D(jd, dd)))
            return kd;

    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::stick_U(int id, int dd)
{
    int jd, kd;

    /* Find a widget above widget dd. */

    if (vert_test(dd, id))
    {
        return id;
    }
    for (jd = m_widgets[id].car; jd; jd = m_widgets[jd].cdr)
    {
        if ((kd = stick_U(jd, dd)))
        {
            return kd;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
int WidgetSet::stick(int id, int axis, int dir, int value)
{
    /* Flag the axes to prevent uncontrolled scrolling. */

    static int x_not_pressed = 1;
    static int y_not_pressed = 1;

    int jd = 0;

    /* Find a new active widget in the direction of joystick motion. */

    if (axis == 0)
    {
        if(!value) x_not_pressed = 1;
        else if(dir == 0 && x_not_pressed)
        {
            jd = stick_L(id, m_active);
            x_not_pressed = 0;
        }
        else if (dir == 1 && x_not_pressed)
        {
            jd = stick_R(id, m_active);
            x_not_pressed = 0;
        }
    }
    else if(axis == 1)
    {
        if(!value) y_not_pressed = 1;
        else if(dir == 0 && y_not_pressed)
        {
            {
                jd = stick_U(id, m_active);
                // Skip over GUI_SPACE
                while(jd && !hot(jd)) jd = stick_U(id, jd);
                y_not_pressed = 0;
            }
        }
        else if (dir == 1 && y_not_pressed)
        {
            jd = stick_D(id, m_active);
            // Skip over GUI_SPACE
            while(jd && !hot(jd)) jd = stick_D(id, jd);
            y_not_pressed = 0;
        }
    }

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == m_active)
        return 0;
    else
        return m_active = jd;
}

//-----------------------------------------------------------------------------
int WidgetSet::cursor(int id, int key)
{
    int jd = 0;

    switch (key)
    {
    case SDLK_LEFT:  jd = stick_L(id, m_active); break;
    case SDLK_RIGHT: jd = stick_R(id, m_active); break;
    case SDLK_UP:    jd = stick_U(id, m_active);
        // Skip over GUI_SPACE
        while( jd && !hot(jd)) jd = stick_U(id, jd);
        break;
    case SDLK_DOWN:  jd = stick_D(id, m_active);
        // Skip over GUI_SPACE
        while (jd && !hot(jd)) jd = stick_D(id, jd);
        break;
    default: return 0;
    }

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == m_active)
        return 0;
    else
        return m_active = jd;
}

//-----------------------------------------------------------------------------
void WidgetSet::set_active(int id)
{
    m_active = id;
}

//-----------------------------------------------------------------------------
void WidgetSet::tgl_paused()
{
    if (m_paused)
    {
        sound_manager -> resumeMusic() ;
        m_paused = false;
    }
    else
    {
        sound_manager -> pauseMusic() ;
        m_paused = true;
    }
}

//-----------------------------------------------------------------------------
void WidgetSet::config_push_persp(float fov, float n, float f)
{
    GLdouble m[4][4];

    const GLdouble R = fov / 2 * M_PI / 180;
    const GLdouble S = sin(R);
    const GLdouble C = cos(R) / S;

    const GLdouble A = (GLdouble)config->m_width/(GLdouble)config->m_height;

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();

        m[0][0] =  C/A;
        m[0][1] =  0.0;
        m[0][2] =  0.0;
        m[0][3] =  0.0;
        m[1][0] =  0.0;
        m[1][1] =    C;
        m[1][2] =  0.0;
        m[1][3] =  0.0;
        m[2][0] =  0.0;
        m[2][1] =  0.0;
        m[2][2] = -(f + n) / (f - n);
        m[2][3] = -1.0;
        m[3][0] =  0.0;
        m[3][1] =  0.0;
        m[3][2] = -2.0 * n * f / (f - n);
        m[3][3] =  0.0;

        glMultMatrixd(&m[0][0]);
    }
    glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------
void WidgetSet::config_push_ortho()
{
    const GLdouble W = (GLdouble) config->m_width;
    const GLdouble H = (GLdouble) config->m_height;

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, W, 0.0, H, -1.0, +1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------
void WidgetSet::config_pop_matrix()
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

