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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "sdldrv.h"
#include "WidgetSet.h"
#include "widget_image.h"

#include "tuxkart.h"
#include "Loader.h"
#include "sound.h"


WidgetSet::WidgetSet()
	: active(0), pause_id(0), paused(0)
{
	font[0] = NULL;
	font[1] = NULL;
	font[2] = NULL;

	const float *c0 = gui_yel;
	const float *c1 = gui_red;

	int w = getScreenWidth();
	int h = getScreenHeight();
	int i, j, s = (h < w) ? h : w;

	/* Initialize font rendering. */

	if (TTF_Init() == 0)
	{
		memset(widgets, 0, sizeof (Widget) * MAXWIDGETS);

		/* Load small, medium, and large typefaces. */
		std::string path = loader->getPath(GUI_FACE);

		//Neverball sizes:
		//font[GUI_SML] = TTF_OpenFont(path.c_str(), s / 24);
		//font[GUI_MED] = TTF_OpenFont(path.c_str(), s / 12);
		//font[GUI_LRG] = TTF_OpenFont(path.c_str(), s /  6);
		
		font[GUI_SML] = TTF_OpenFont(path.c_str(), s / 24);
		font[GUI_MED] = TTF_OpenFont(path.c_str(), s / 15);
		font[GUI_LRG] = TTF_OpenFont(path.c_str(), s /  10);
		radius = s / 60;

		/* Initialize the global pause GUI. */

		if ((pause_id = pause(0)))
			layout(pause_id, 0, 0);

		/* Initialize digit glyphs and lists for counters and clocks. */

		for (i = 0; i < 3; i++)
		{
			char text[2];

			/* Draw digits 0 throught 9. */

			for (j = 0; j < 10; j++)
			{
				text[0] = '0' + (char) j;
				text[1] =  0;

				digit_text[i][j] = make_image_from_font(NULL, NULL,
				                                        &digit_w[i][j],
				                                        &digit_h[i][j],
				                                        text, font[i]);
				digit_list[i][j] = list(-digit_w[i][j] / 2,
				                        -digit_h[i][j] / 2,
				                        +digit_w[i][j],
				                        +digit_h[i][j], c0, c1);
			}

			/* Draw the colon for the clock. */

			digit_text[i][j] = make_image_from_font(NULL, NULL,
			                                        &digit_w[i][10],
			                                        &digit_h[i][10],
			                                        ":", font[i]);
			digit_list[i][j] = list(-digit_w[i][10] / 2,
			                        -digit_h[i][10] / 2,
			                        +digit_w[i][10],
			                        +digit_h[i][10], c0, c1);
		}
	}
}

WidgetSet::~WidgetSet()
{
	int i, j, id;

	/* Release any remaining widget texture and display list indices. */

	for (id = 1; id < MAXWIDGETS; id++)
	{
		if (glIsTexture(widgets[id].text_img))
			glDeleteTextures(1, &widgets[id].text_img);

		if (glIsList(widgets[id].text_obj))
			glDeleteLists(widgets[id].text_obj, 1);
		if (glIsList(widgets[id].rect_obj))
			glDeleteLists(widgets[id].rect_obj, 1);

		widgets[id].type     = GUI_FREE;
		widgets[id].text_img = 0;
		widgets[id].text_obj = 0;
		widgets[id].rect_obj = 0;
		widgets[id].cdr      = 0;
		widgets[id].car      = 0;
	}

	/* Release all digit textures and display lists. */

	for (i = 0; i < 3; i++)
		for (j = 0; j < 11; j++)
		{
			if (glIsTexture(digit_text[i][j]))
				glDeleteTextures(1, &digit_text[i][j]);

			if (glIsList(digit_list[i][j]))
				glDeleteLists(digit_list[i][j], 1);
		}

	/* Release all loaded fonts and finalize font rendering. */

	if (font[GUI_LRG]) TTF_CloseFont(font[GUI_LRG]);
	if (font[GUI_MED]) TTF_CloseFont(font[GUI_MED]);
	if (font[GUI_SML]) TTF_CloseFont(font[GUI_SML]);

	TTF_Quit();
}



int WidgetSet::hot(int id)
{
	return (widgets[id].type & GUI_STATE);
}


/*---------------------------------------------------------------------------*/
/*
 * Initialize a  display list  containing a  rectangle (x, y, w, h) to
 * which a  rendered-font texture  may be applied.   Colors  c0 and c1
 * determine the top-to-bottom color gradiant of the text.
 */

GLuint WidgetSet::list(int x, int y, int w, int h, const float *c0, const float *c1)
{
	GLuint list = glGenLists(1);

	GLfloat s0, t0;
	GLfloat s1, t1;

	int W, H, d = h / 16;

	/* Assume the applied texture size is rect size rounded to power-of-two. */

	image_size(&W, &H, w, h);

	s0 = 0.5f * (W - w) / W;
	t0 = 0.5f * (H - h) / H;
	s1 = 1.0f - s0;
	t1 = 1.0f - t0;

	glNewList(list, GL_COMPILE);
	{
		glBegin(GL_QUADS);
		{
			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			glTexCoord2f(s0, t1); glVertex2i(x     + d, y     - d);
			glTexCoord2f(s1, t1); glVertex2i(x + w + d, y     - d);
			glTexCoord2f(s1, t0); glVertex2i(x + w + d, y + h - d);
			glTexCoord2f(s0, t0); glVertex2i(x     + d, y + h - d);

			glColor4fv(c0);
			glTexCoord2f(s0, t1); glVertex2i(x,     y);
			glTexCoord2f(s1, t1); glVertex2i(x + w, y);

			glColor4fv(c1);
			glTexCoord2f(s1, t0); glVertex2i(x + w, y + h);
			glTexCoord2f(s0, t0); glVertex2i(x,     y + h);
		}
		glEnd();
	}
	glEndList();

	return list;
}


/*
 * Initialize a display list containing a rounded-corner rectangle (x,
 * y, w, h).  Generate texture coordinates to properly apply a texture
 * map to the rectangle as though the corners were not rounded.
 */

GLuint WidgetSet::rect(int x, int y, int w, int h, int f, int r)
{
    GLuint list = glGenLists(1);

    int n = 8;
    int i;

    glNewList(list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            /* Left side... */

            for (i = 0; i <= n; i++)
            {
                float a = 0.5f * M_PI * (float) i / (float) n;
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

            for (i = 0; i <= n; i++)
            {
                float a = 0.5f * M_PI * (float) i / (float) n;
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



/*---------------------------------------------------------------------------*/


int WidgetSet::add_widget(int pd, int type)
{
    int id;

    /* Find an unused entry in the widget table. */

    for (id = 1; id < MAXWIDGETS; id++)
        if (widgets[id].type == GUI_FREE)
        {
            /* Set the type and default properties. */

            widgets[id].type     = type;
            widgets[id].token    = 0;
            widgets[id].value    = 0;
            widgets[id].size     = 0;
            widgets[id].rect     = GUI_NW | GUI_SW | GUI_NE | GUI_SE;
            widgets[id].w        = 0;
            widgets[id].h        = 0;
            widgets[id].text_img = 0;
            widgets[id].text_obj = 0;
            widgets[id].rect_obj = 0;
            widgets[id].color0   = gui_wht;
            widgets[id].color1   = gui_wht;
            widgets[id].scale    = 1.0f;

            /* Insert the new widget into the parents's widget list. */

            if (pd)
            {
                widgets[id].car = 0;
                widgets[id].cdr = widgets[pd].car;
                widgets[pd].car = id;
            }
            else
            {
                widgets[id].car = 0;
                widgets[id].cdr = 0;
            }

            return id;
        }

    fprintf(stderr, "Out of widget IDs\n");

    return 0;
}

int WidgetSet::harray(int pd) { return add_widget(pd, GUI_HARRAY); }
int WidgetSet::varray(int pd) { return add_widget(pd, GUI_VARRAY); }
int WidgetSet::hstack(int pd) { return add_widget(pd, GUI_HSTACK); }
int WidgetSet::vstack(int pd) { return add_widget(pd, GUI_VSTACK); }
int WidgetSet::filler(int pd) { return add_widget(pd, GUI_FILLER); }

/*---------------------------------------------------------------------------*/

void WidgetSet::set_label(int id, const char *text)
{
    int w, h;

    if (glIsTexture(widgets[id].text_img))
        glDeleteTextures(1, &widgets[id].text_img);
    if (glIsList(widgets[id].text_obj))
        glDeleteLists(widgets[id].text_obj, 1);

    widgets[id].text_img = make_image_from_font(NULL, NULL, &w, &h,
                                               text, font[widgets[id].size]);
    widgets[id].text_obj = list(-w / 2, -h / 2, w, h,
                                   widgets[id].color0, widgets[id].color1);
}

void WidgetSet::set_image(int id, const char *file)
{
    if (glIsTexture(widgets[id].text_img))
        glDeleteTextures(1, &widgets[id].text_img);

    widgets[id].text_img = make_image_from_file(NULL, NULL, NULL, NULL, file);
}

void WidgetSet::set_count(int id, int value)
{
    widgets[id].value = value;
}

void WidgetSet::set_clock(int id, int value)
{
    widgets[id].value = value;
}

void WidgetSet::set_multi(int id, const char *text)
{
    const char *p;

    char s[8][MAXSTR];
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

    for (i = j - 1, jd = widgets[id].car; i >= 0 && jd; i--, jd = widgets[jd].cdr)
        set_label(jd, s[i]);
}

/*---------------------------------------------------------------------------*/

int WidgetSet::image(int pd, const char *file, int w, int h)
{
    int id;

    if ((id = add_widget(pd, GUI_IMAGE)))
    {
        widgets[id].text_img = make_image_from_file(NULL, NULL,
                                                   NULL, NULL, file);
        widgets[id].w     = w;
        widgets[id].h     = h;
    }
    return id;
}

int WidgetSet::start(int pd, const char *text, int size, int token, int value)
{
    int id;

    if ((id = state(pd, text, size, token, value)))
        active = id;

    return id;
}

int WidgetSet::state(int pd, const char *text, int size, int token, int value)
{
    int id;

    if ((id = add_widget(pd, GUI_STATE)))
    {
        widgets[id].text_img = make_image_from_font(NULL, NULL,
                                                   &widgets[id].w,
                                                   &widgets[id].h,
                                                   text, font[size]);
        widgets[id].size  = size;
        widgets[id].token = token;
        widgets[id].value = value;
    }
    return id;
}

int WidgetSet::label(int pd, const char *text, int size, int rect, const float *c0,
                                                            const float *c1)
{
    int id;

    if ((id = add_widget(pd, GUI_LABEL)))
    {
        widgets[id].text_img = make_image_from_font(NULL, NULL,
                                                   &widgets[id].w,
                                                   &widgets[id].h,
                                                   text, font[size]);
        widgets[id].size   = size;
        widgets[id].color0 = c0 ? c0 : gui_yel;
        widgets[id].color1 = c1 ? c1 : gui_red;
        widgets[id].rect   = rect;
    }
    return id;
}

int WidgetSet::count(int pd, int value, int size, int rect)
{
    int i, id;

    if ((id = add_widget(pd, GUI_COUNT)))
    {
        for (i = value; i; i /= 10)
            widgets[id].w += digit_w[size][0];

        widgets[id].h      = digit_h[size][0];
        widgets[id].value  = value;
        widgets[id].size   = size;
        widgets[id].color0 = gui_yel;
        widgets[id].color1 = gui_red;
        widgets[id].rect   = rect;
    }
    return id;
}

int WidgetSet::clock(int pd, int value, int size, int rect)
{
    int id;

    if ((id = add_widget(pd, GUI_CLOCK)))
    {
        widgets[id].w      = digit_w[size][0] * 6;
        widgets[id].h      = digit_h[size][0];
        widgets[id].value  = value;
        widgets[id].size   = size;
        widgets[id].color0 = gui_yel;
        widgets[id].color1 = gui_red;
        widgets[id].rect   = rect;
    }
    return id;
}

int WidgetSet::space(int pd)
{
    int id;

    if ((id = add_widget(pd, GUI_SPACE)))
    {
        widgets[id].w = 0;
        widgets[id].h = 0;
    }
    return id;
}

int WidgetSet::pause(int pd)
{
    int id;

    if ((id = add_widget(pd, GUI_PAUSE)))
    {
        widgets[id].value  = 0;
        widgets[id].rect   = GUI_ALL;
    }
    return id;
}

/*---------------------------------------------------------------------------*/
/*
 * Create  a multi-line  text box  using a  vertical array  of labels.
 * Parse the  text for '\'  characters and treat them  as line-breaks.
 * Preserve the rect specifation across the entire array.
 */

int WidgetSet::multi(int pd, const char *text, int size, int rect, const float *c0,
                                                            const float *c1)
{
    int id = 0;

    if (text && (id = varray(pd)))
    {
        const char *p;

        char s[8][MAXSTR];
        int  r[8];
        int  i, j;

        size_t n = 0;

        /* Copy each delimited string to a line buffer. */

        for (p = text, j = 0; *p && j < 8; j++)
        {
            strncpy(s[j], p, (n = strcspn(p, "\\")));
            s[j][n] = 0;
            r[j]    = 0;

            if (*(p += n) == '\\') p++;
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

/*---------------------------------------------------------------------------*/
/*
 * The bottom-up pass determines the area of all widgets.  The minimum
 * width  and height of  a leaf  widget is  given by  the size  of its
 * contents.   Array  and  stack   widths  and  heights  are  computed
 * recursively from these.
 */

void WidgetSet::harray_up(int id)
{
    int jd, c = 0;

    /* Find the widest child width and the highest child height. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
    {
        widget_up(jd);

        if (widgets[id].h < widgets[jd].h)
            widgets[id].h = widgets[jd].h;
        if (widgets[id].w < widgets[jd].w)
            widgets[id].w = widgets[jd].w;
        c++;
    }

    /* Total width is the widest child width times the child count. */

    widgets[id].w *= c;
}

void WidgetSet::varray_up(int id)
{
    int jd, c = 0;

    /* Find the widest child width and the highest child height. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
    {
        widget_up(jd);

        if (widgets[id].h < widgets[jd].h)
            widgets[id].h = widgets[jd].h;
        if (widgets[id].w < widgets[jd].w)
            widgets[id].w = widgets[jd].w;

        c++;
    }

    /* Total height is the highest child height times the child count. */

    widgets[id].h *= c;
}

void WidgetSet::hstack_up(int id)
{
    int jd;

    /* Find the highest child height.  Sum the child widths. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
    {
        widget_up(jd);

        if (widgets[id].h < widgets[jd].h)
            widgets[id].h = widgets[jd].h;

        widgets[id].w += widgets[jd].w;
    }
}

void WidgetSet::vstack_up(int id)
{
    int jd;

    /* Find the widest child width.  Sum the child heights. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
    {
        widget_up(jd);

        if (widgets[id].w < widgets[jd].w)
            widgets[id].w = widgets[jd].w;

        widgets[id].h += widgets[jd].h;
    }
}

void WidgetSet::paused_up(int id)
{
    /* Store width and height for later use in text rendering. */

    widgets[id].x = widgets[id].w;
    widgets[id].y = widgets[id].h;

    /* The pause widget fills the screen. */

    widgets[id].w = getScreenWidth();
    widgets[id].h = getScreenHeight();
}

void WidgetSet::button_up(int id)
{
    /* Store width and height for later use in text rendering. */

    widgets[id].x = widgets[id].w;
    widgets[id].y = widgets[id].h;

    if (widgets[id].w < widgets[id].h && widgets[id].w > 0)
        widgets[id].w = widgets[id].h;


    /* Padded text elements look a little nicer. */

    if (widgets[id].w < getScreenWidth())
        widgets[id].w += radius;
    if (widgets[id].h < getScreenWidth())
        widgets[id].h += radius;
}

void WidgetSet::widget_up(int id)
{
    if (id)
        switch (widgets[id].type & GUI_TYPE)
        {
        case GUI_HARRAY: harray_up(id); break;
        case GUI_VARRAY: varray_up(id); break;
        case GUI_HSTACK: hstack_up(id); break;
        case GUI_VSTACK: vstack_up(id); break;
        case GUI_PAUSE:  paused_up(id); break;
        default:         button_up(id); break;
        }
}

/*---------------------------------------------------------------------------*/
/*
 * The  top-down layout  pass distributes  available area  as computed
 * during the bottom-up pass.  Widgets  use their area and position to
 * initialize rendering state.
 */

void WidgetSet::harray_dn(int id, int x, int y, int w, int h)
{
    int jd, i = 0, c = 0;

    widgets[id].x = x;
    widgets[id].y = y;
    widgets[id].w = w;
    widgets[id].h = h;

    /* Count children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        c += 1;

    /* Distribute horizontal space evenly to all children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr, i++)
    {
        int x0 = x +  i      * w / c;
        int x1 = x + (i + 1) * w / c;

        widget_dn(jd, x0, y, x1 - x0, h);
    }
}

void WidgetSet::varray_dn(int id, int x, int y, int w, int h)
{
    int jd, i = 0, c = 0;

    widgets[id].x = x;
    widgets[id].y = y;
    widgets[id].w = w;
    widgets[id].h = h;

    /* Count children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        c += 1;

    /* Distribute vertical space evenly to all children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr, i++)
    {
        int y0 = y +  i      * h / c;
        int y1 = y + (i + 1) * h / c;

        widget_dn(jd, x, y0, w, y1 - y0);
    }
}

void WidgetSet::hstack_dn(int id, int x, int y, int w, int h)
{
    int jd, jx = x, jw = 0, c = 0;

    widgets[id].x = x;
    widgets[id].y = y;
    widgets[id].w = w;
    widgets[id].h = h;

    /* Measure the total width requested by non-filler children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        if ((widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            c += 1;
        else
            jw += widgets[jd].w;

    /* Give non-filler children their requested space.   */
    /* Distribute the rest evenly among filler children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
    {
        if ((widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            widget_dn(jd, jx, y, (w - jw) / c, h);
        else
            widget_dn(jd, jx, y, widgets[jd].w, h);

        jx += widgets[jd].w;
    }
}

void WidgetSet::vstack_dn(int id, int x, int y, int w, int h)
{
    int jd, jy = y, jh = 0, c = 0;

    widgets[id].x = x;
    widgets[id].y = y;
    widgets[id].w = w;
    widgets[id].h = h;

    /* Measure the total height requested by non-filler children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        if ((widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            c += 1;
        else
            jh += widgets[jd].h;

    /* Give non-filler children their requested space.   */
    /* Distribute the rest evenly among filler children. */

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
    {
        if ((widgets[jd].type & GUI_TYPE) == GUI_FILLER)
            widget_dn(jd, x, jy, w, (h - jh) / c);
        else
            widget_dn(jd, x, jy, w, widgets[jd].h);

        jy += widgets[jd].h;
    }
}

void WidgetSet::filler_dn(int id, int x, int y, int w, int h)
{
    /* Filler expands to whatever size it is given. */

    widgets[id].x = x;
    widgets[id].y = y;
    widgets[id].w = w;
    widgets[id].h = h;
}

void WidgetSet::button_dn(int id, int x, int y, int w, int h)
{
    /* Recall stored width and height for text rendering. */

    int W = widgets[id].x;
    int H = widgets[id].y;
    int R = widgets[id].rect;
    int r = ((widgets[id].type & GUI_TYPE) == GUI_PAUSE ? radius * 4 : radius);

    const float *c0 = widgets[id].color0;
    const float *c1 = widgets[id].color1;

    widgets[id].x = x;
    widgets[id].y = y;
    widgets[id].w = w;
    widgets[id].h = h;

    /* Create display lists for the text area and rounded rectangle. */

    widgets[id].text_obj = list(-W / 2, -H / 2, W, H, c0, c1);
    widgets[id].rect_obj = rect(-w / 2, -h / 2, w, h, R, r);
}

void WidgetSet::widget_dn(int id, int x, int y, int w, int h)
{
    if (id)
        switch (widgets[id].type & GUI_TYPE)
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

/*---------------------------------------------------------------------------*/
/*
 * During GUI layout, we make a bottom-up pass to determine total area
 * requirements for  the widget  tree.  We position  this area  to the
 * sides or center of the screen.  Finally, we make a top-down pass to
 * distribute this area to each widget.
 */

void WidgetSet::layout(int id, int xd, int yd)
{
    int x, y;

    int w, W = getScreenWidth();
    int h, H = getScreenHeight();

    widget_up(id);

    w = widgets[id].w;
    h = widgets[id].h;

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

int WidgetSet::search(int id, int x, int y)
{
    int jd, kd;
    assert(id < MAXWIDGETS);
    
    /* Search the hierarchy for the widget containing the given point. */

    if (id && (widgets[id].x <= x && x < widgets[id].x + widgets[id].w &&
               widgets[id].y <= y && y < widgets[id].y + widgets[id].h))
    {
        if (hot(id))
            return id;

        for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
            if ((kd = search(jd, x, y)))
                return kd;
    }
    return 0;
}

/*
 * Activate a widget, allowing it  to behave as a normal state widget.
 * This may  be used  to create  image buttons, or  cause an  array of
 * widgets to behave as a single state widget.
 */
int WidgetSet::activate_widget(int id, int token, int value)
{
    widgets[id].type |= GUI_STATE;
    widgets[id].token = token;
    widgets[id].value = value;

    return id;
}

int WidgetSet::delete_widget(int id)
{
    if (id)
    {
        /* Recursively delete all subwidgets. */

        delete_widget(widgets[id].cdr);
        delete_widget(widgets[id].car);

        /* Release any GL resources held by this widget. */

        if (glIsTexture(widgets[id].text_img))
            glDeleteTextures(1, &widgets[id].text_img);

        if (glIsList(widgets[id].text_obj))
            glDeleteLists(widgets[id].text_obj, 1);
        if (glIsList(widgets[id].rect_obj))
            glDeleteLists(widgets[id].rect_obj, 1);

        /* Mark this widget unused. */

        widgets[id].type     = GUI_FREE;
        widgets[id].text_img = 0;
        widgets[id].text_obj = 0;
        widgets[id].rect_obj = 0;
        widgets[id].cdr      = 0;
        widgets[id].car      = 0;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void WidgetSet::paint_rect(int id, int st)
{
#ifdef SNIP
    static const GLfloat back[4][4] = {
        { 0.1f, 0.1f, 0.1f, 0.5f },             /* off and inactive    */
        { 0.3f, 0.3f, 0.3f, 0.5f },             /* off and   active    */
        { 0.7f, 0.3f, 0.0f, 0.5f },             /* on  and inactive    */
        { 1.0f, 0.7f, 0.3f, 0.5f },             /* on  and   active    */
    };
#endif
    static const GLfloat back[4][4] = {
        { 0.1f, 0.1f, 0.1f, 0.5f },             /* off and inactive    */
        { 0.5f, 0.5f, 0.5f, 0.8f },             /* off and   active    */
        { 1.0f, 0.7f, 0.3f, 0.5f },             /* on  and inactive    */
        { 1.0f, 0.7f, 0.3f, 0.8f },             /* on  and   active    */
    };

    int jd, i = 0;

    /* Use the widget status to determine the background color. */

    if (hot(id))
        i = st | (((widgets[id].value) ? 2 : 0) |
                  ((id == active)     ? 1 : 0));

    switch (widgets[id].type & GUI_TYPE)
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

        for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
            paint_rect(jd, i);

        break;

    default:

        /* Draw a leaf's background, colored by widget state. */

        glPushMatrix();
        {
            glTranslatef((GLfloat) (widgets[id].x + widgets[id].w / 2),
                         (GLfloat) (widgets[id].y + widgets[id].h / 2), 0.f);

            glColor4fv(back[i]);
            glCallList(widgets[id].rect_obj);
        }
        glPopMatrix();

        break;
    }
}

/*---------------------------------------------------------------------------*/

void WidgetSet::paint_array(int id)
{
    int jd;

    glPushMatrix();
    {
        GLfloat cx = widgets[id].x + widgets[id].w / 2.0f;
        GLfloat cy = widgets[id].y + widgets[id].h / 2.0f;
        GLfloat ck = widgets[id].scale;

        glTranslatef(+cx, +cy, 0.0f);
        glScalef(ck, ck, ck);
        glTranslatef(-cx, -cy, 0.0f);

        /* Recursively paint all subwidgets. */

        for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
            paint_text(jd);
    }
    glPopMatrix();
}

void WidgetSet::paint_image(int id)
{
    /* Draw the widget rect, textured using the image. */

    glPushMatrix();
    {
        glTranslatef((GLfloat) (widgets[id].x + widgets[id].w / 2),
                     (GLfloat) (widgets[id].y + widgets[id].h / 2), 0.f);

        glScalef(widgets[id].scale,
                 widgets[id].scale,
                 widgets[id].scale);

        glBindTexture(GL_TEXTURE_2D, widgets[id].text_img);
        glColor4fv(gui_wht);
        glCallList(widgets[id].rect_obj);
    }
    glPopMatrix();
}

void WidgetSet::paint_count(int id)
{
    int j, i = widgets[id].size;

    glPushMatrix();
    {
        glColor4fv(gui_wht);

        /* Translate to the widget center, and apply the pulse scale. */

        glTranslatef((GLfloat) (widgets[id].x + widgets[id].w / 2),
                     (GLfloat) (widgets[id].y + widgets[id].h / 2), 0.f);

        glScalef(widgets[id].scale,
                 widgets[id].scale,
                 widgets[id].scale);

        if (widgets[id].value)
        {
            /* Translate left by half the total width of the rendered value. */

            for (j = widgets[id].value; j; j /= 10)
                glTranslatef((GLfloat) +digit_w[i][j % 10] / 2.0f, 0.0f, 0.0f);

            glTranslatef((GLfloat) -digit_w[i][0] / 2.0f, 0.0f, 0.0f);

            /* Render each digit, moving right after each. */

            for (j = widgets[id].value; j; j /= 10)
            {
                glBindTexture(GL_TEXTURE_2D, digit_text[i][j % 10]);
                glCallList(digit_list[i][j % 10]);
                glTranslatef((GLfloat) -digit_w[i][j % 10], 0.0f, 0.0f);
            }
        }
        else
        {
            /* If the value is zero, just display a zero in place. */

            glBindTexture(GL_TEXTURE_2D, digit_text[i][0]);
            glCallList(digit_list[i][0]);
        }
    }
    glPopMatrix();
}

void WidgetSet::paint_clock(int id)
{
    int i  =   widgets[id].size;
    int mt =  (widgets[id].value / 6000) / 10;
    int mo =  (widgets[id].value / 6000) % 10;
    int st = ((widgets[id].value % 6000) / 100) / 10;
    int so = ((widgets[id].value % 6000) / 100) % 10;
    int ht = ((widgets[id].value % 6000) % 100) / 10;
    int ho = ((widgets[id].value % 6000) % 100) % 10;

    GLfloat dx_large = (GLfloat) digit_w[i][0];
    GLfloat dx_small = (GLfloat) digit_w[i][0] * 0.75f;

    glPushMatrix();
    {
        glColor4fv(gui_wht);

        /* Translate to the widget center, and apply the pulse scale. */

        glTranslatef((GLfloat) (widgets[id].x + widgets[id].w / 2),
                     (GLfloat) (widgets[id].y + widgets[id].h / 2), 0.f);

        glScalef(widgets[id].scale,
                 widgets[id].scale,
                 widgets[id].scale);

        /* Translate left by half the total width of the rendered value. */

        if (mt > 0)
            glTranslatef(-2.25f * dx_large, 0.0f, 0.0f);
        else
            glTranslatef(-1.75f * dx_large, 0.0f, 0.0f);

        /* Render the minutes counter. */

        if (mt > 0)
        {
            glBindTexture(GL_TEXTURE_2D, digit_text[i][mt]);
            glCallList(digit_list[i][mt]);
            glTranslatef(dx_large, 0.0f, 0.0f);
        }

        glBindTexture(GL_TEXTURE_2D, digit_text[i][mo]);
        glCallList(digit_list[i][mo]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render the colon. */

        glBindTexture(GL_TEXTURE_2D, digit_text[i][10]);
        glCallList(digit_list[i][10]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render the seconds counter. */

        glBindTexture(GL_TEXTURE_2D, digit_text[i][st]);
        glCallList(digit_list[i][st]);
        glTranslatef(dx_large, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, digit_text[i][so]);
        glCallList(digit_list[i][so]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render hundredths counter half size. */

        glScalef(0.5f, 0.5f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, digit_text[i][ht]);
        glCallList(digit_list[i][ht]);
        glTranslatef(dx_large, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, digit_text[i][ho]);
        glCallList(digit_list[i][ho]);
    }
    glPopMatrix();
}

void WidgetSet::paint_label(int id)
{
    /* Draw the widget text box, textured using the glyph. */

    glPushMatrix();
    {
        glTranslatef((GLfloat) (widgets[id].x + widgets[id].w / 2),
                     (GLfloat) (widgets[id].y + widgets[id].h / 2), 0.f);

        glScalef(widgets[id].scale,
                 widgets[id].scale,
                 widgets[id].scale);

        glBindTexture(GL_TEXTURE_2D, widgets[id].text_img);
        glCallList(widgets[id].text_obj);
    }
    glPopMatrix();
}

void WidgetSet::paint_text(int id)
{
    switch (widgets[id].type & GUI_TYPE)
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

	glPushAttrib(GL_TEXTURE_BIT);
	{
		glDisable(GL_TEXTURE_2D);
		paint_rect(id, 0);
	}
	glPopAttrib();

	//this isn't here in the Neverball code, it's somewhere in Neverball's main code, however it is neccessary
	//for fonts to be displayed
	glEnable(GL_TEXTURE_2D);
	
	paint_text(id);

        config_pop_matrix();
        glPopAttrib();
    }
}

void WidgetSet::blank()
{
    paint(pause_id);
}

/*---------------------------------------------------------------------------*/

void WidgetSet::dump(int id, int d)
{
    int jd, i;

    if (id)
    {
        char *type = "?";

        switch (widgets[id].type & GUI_TYPE)
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

        for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
            dump(jd, d + 1);
    }
}

void WidgetSet::pulse(int id, float k)
{
    if (id) widgets[id].scale = k;
}

void WidgetSet::timer(int id, float dt)
{
    int jd;

    if (id)
    {
        for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
            timer(jd, dt);

        if (widgets[id].scale - 1.0f < dt)
            widgets[id].scale = 1.0f;
        else
            widgets[id].scale -= dt;
    }
}

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

    jd = search(active, x, y);

    /* If not still active, search the hierarchy for a new active widget. */

    if (jd == 0)
        jd = search(id, x, y);

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == active)
        return 0;
    else
        return active = jd;
}

int WidgetSet::click(void)
{
    return active;
}

int WidgetSet::token(int id) const
{
    return id ? widgets[id].token : 0;
}

int WidgetSet::value(int id) const
{
    return id ? widgets[id].value : 0;
}

void WidgetSet::toggle(int id)
{
    widgets[id].value = widgets[id].value ? 0 : 1;
}

/*---------------------------------------------------------------------------*/

int WidgetSet::vert_test(int id, int jd)
{
	/* Determine whether widget id is in vertical contact with widget jd. */

	if (id && hot(id) && jd && hot(jd))
	{
		int i0 = widgets[id].x;
		int i1 = widgets[id].x + widgets[id].w;
		int j0 = widgets[jd].x;
		int j1 = widgets[jd].x + widgets[jd].w;

		/* Is widget id's top edge is in contact with jd's bottom edge? */

		if (widgets[id].y + widgets[id].h == widgets[jd].y)
		{
			/* Do widgets id and jd overlap horizontally? */

			if (j0 <= i0 && i0 <  j1) return 1;
			if (j0 <  i1 && i1 <= j1) return 1;
			if (i0 <= j0 && j0 <  i1) return 1;
			if (i0 <  j1 && j1 <= i1) return 1;
		}
	}
	return 0;
}

int WidgetSet::horz_test(int id, int jd)
{
    /* Determine whether widget id is in horizontal contact with widget jd. */

    if (id && hot(id) && jd && hot(jd))
    {
        int i0 = widgets[id].y;
        int i1 = widgets[id].y + widgets[id].h;
        int j0 = widgets[jd].y;
        int j1 = widgets[jd].y + widgets[jd].h;

        /* Is widget id's right edge in contact with jd's left edge? */

        if (widgets[id].x + widgets[id].w == widgets[jd].x)
        {
            /* Do widgets id and jd overlap vertically? */

            if (j0 <= i0 && i0 <  j1) return 1;
            if (j0 <  i1 && i1 <= j1) return 1;
            if (i0 <= j0 && j0 <  i1) return 1;
            if (i0 <  j1 && j1 <= i1) return 1;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int WidgetSet::stick_L(int id, int dd)
{
    int jd, kd;

    /* Find a widget to the left of widget dd. */

    if (horz_test(id, dd))
        return id;

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        if ((kd = stick_L(jd, dd)))
            return kd;

    return 0;
}

int WidgetSet::stick_R(int id, int dd)
{
    int jd, kd;

    /* Find a widget to the right of widget dd. */

    if (horz_test(dd, id))
        return id;

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        if ((kd = stick_R(jd, dd)))
            return kd;

    return 0;
}

int WidgetSet::stick_D(int id, int dd)
{
	int jd, kd;

	/* Find a widget below widget dd. */

	if (vert_test(id, dd))
		return id;

	for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
		if ((kd = stick_D(jd, dd)))
			return kd;

	return 0;
}

int WidgetSet::stick_U(int id, int dd)
{
    int jd, kd;

    /* Find a widget above widget dd. */

    if (vert_test(dd, id))
        return id;

    for (jd = widgets[id].car; jd; jd = widgets[jd].cdr)
        if ((kd = stick_U(jd, dd)))
            return kd;

    return 0;
}


int WidgetSet::stick(int id, int whichAxis, int value)
{
    /* Flag the axes to prevent uncontrolled scrolling. */

    static int xflag = 1;
    static int yflag = 1;

    int jd = 0;

    /* Find a new active widget in the direction of joystick motion. */
    
    if (whichAxis == 0)
    {
        if (-JOY_MID <= value && value <= +JOY_MID)
            xflag = 1;
        else if (value < -JOY_MID && xflag && (jd = stick_L(id, active)))
            xflag = 0;
        else if (value > +JOY_MID && xflag && (jd = stick_R(id, active)))
            xflag = 0;
    }
    
    else //whichAxis must equal 1
    {
        if (-JOY_MID <= value && value <= +JOY_MID)
            yflag = 1;
        else if (value < -JOY_MID && yflag && (jd = stick_U(id, active)))
            yflag = 0;
        else if (value > +JOY_MID && yflag && (jd = stick_D(id, active)))
            yflag = 0;
    }

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == active)
        return 0;
    else
        return active = jd;
}

int WidgetSet::cursor(int id, SDLKey key)
{
	int jd = 0;
	
	switch (key)
	{
	case SDLK_LEFT: jd = stick_L(id, active); break;
	case SDLK_RIGHT: jd = stick_R(id, active); break;
	case SDLK_UP: jd = stick_U(id, active); break;
	case SDLK_DOWN: jd = stick_D(id, active); break;
	default: return 0;
	}
	
    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == active)
        return 0;
    else
        return active = jd;
}

void WidgetSet::set_active(int id)
{
	active = id;
}

/*---------------------------------------------------------------------------*/

void WidgetSet::set_paused()
{
    sound -> pause_music() ;
    paused = true;
}

void WidgetSet::clr_paused()
{
	 sound -> resume_music() ;
    paused = false;
}

void WidgetSet::tgl_paused()
{
    if (paused)
        clr_paused();
    else
        set_paused();
}

/*---------------------------------------------------------------------------*/

void WidgetSet::config_push_persp(float fov, float n, float f)
{
    GLdouble m[4][4];

    GLdouble r = fov / 2 * M_PI / 180;
    GLdouble s = sin(r);
    GLdouble c = cos(r) / s;

    GLdouble a = ((GLdouble) getScreenWidth() / 
                  (GLdouble) getScreenHeight());

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();

        m[0][0] =  c/a;
        m[0][1] =  0.0;
        m[0][2] =  0.0;
        m[0][3] =  0.0;
        m[1][0] =  0.0;
        m[1][1] =    c;
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

void WidgetSet::config_push_ortho()
{
    GLdouble w = (GLdouble) getScreenWidth();
    GLdouble h = (GLdouble) getScreenHeight();

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, w, 0.0, h, -1.0, +1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

void WidgetSet::config_pop_matrix()
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

void WidgetSet::config_clear()
{
/* CONFIG REFLECTION not defined in tuxkart, and in fact this entire function is never used. 
All this code is originally from Neverball
    if (option_d[CONFIG_REFLECTION])
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
		    else
		    */
    
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
}
	
/*---------------------------------------------------------------------------*/



