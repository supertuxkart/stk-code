//  $Id: WidgetSet.h,v 1.6 2004/08/17 21:01:17 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#ifndef HEADER_WIDGETSET_H
#define HEADER_WIDGETSET_H

#include "widget_glext.h"
#include "joystick.h"

#include <SDL_ttf.h>

/*---------------------------------------------------------------------------*/

#define GUI_FACE "fonts/VeraBd.ttf"
#define GUI_SML  0
#define GUI_MED  1
#define GUI_LRG  2

#define GUI_NW   1
#define GUI_SW   2
#define GUI_NE   4
#define GUI_SE   8

#define GUI_LFT  (GUI_NW  | GUI_SW)
#define GUI_RGT  (GUI_NE  | GUI_SE)
#define GUI_TOP  (GUI_NW  | GUI_NE)
#define GUI_BOT  (GUI_SW  | GUI_SE)
#define GUI_ALL  (GUI_TOP | GUI_BOT)

/*---------------------------------------------------------------------------*/

#define MAXWIDGETS 256

#define GUI_TYPE 0xFFFE

#define GUI_FREE   0
#define GUI_STATE  1
#define GUI_HARRAY 2
#define GUI_VARRAY 4
#define GUI_HSTACK 6
#define GUI_VSTACK 8
#define GUI_FILLER 10
#define GUI_IMAGE  12
#define GUI_LABEL  14
#define GUI_COUNT  16
#define GUI_CLOCK  18
#define GUI_SPACE  20
#define GUI_PAUSE  22

/*---------------------------------------------------------------------------*/

//previously in config.h:

#define MAXSTR 256

/*---------------------------------------------------------------------------*/

const GLfloat gui_wht[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat gui_yel[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat gui_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat gui_grn[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat gui_blu[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat gui_blk[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat gui_gry[4] = { 0.3f, 0.3f, 0.3f, 1.0f };

struct Widget
{
    int     type;
    int     token;
    int     value;
    int     size;
    int     rect;

    int     x, y;
    int     w, h;
    int     car;
    int     cdr;

    GLuint  text_img;
    GLuint  text_obj;
    GLuint  rect_obj;

    const GLfloat *color0;
    const GLfloat *color1;

    GLfloat  scale;
};

class WidgetSet
{
public:
	WidgetSet();
	~WidgetSet();
	
	/*---------------------------------------------------------------------------*/
	
	/* change the value of variables stored by already-existing widgets*/	
	
	void set_label(int, const char *);
	void set_image(int, const char *);
	void set_multi(int, const char *);
	void set_count(int, int);
	void set_clock(int, int);
	
	/*---------------------------------------------------------------------------*/
	
	/* creation functions, the first argument is the parent id of the new widget (0 if no parent) */
	
	/* arrays, these are used to setup the layout of your widgets*/	
	
	int  harray(int);
	int  varray(int);
	int  hstack(int);
	int  vstack(int);
	int  filler(int);
	
	/* constructors - add new widget of type x */	
	
	int  image(int, const char *, int, int);
	int  start(int, const char *, int, int, int);
	int  state(int, const char *, int, int, int);
        int  label(int pd, const char *text, int size, int rect, const float *c0, const float *c1);

	int  multi(int, const char *, int, int, const float *, const float *);
	int  count(int, int, int, int);
	int  clock(int, int, int, int);
	int  space(int);
	
	/*---------------------------------------------------------------------------*/
	
	/* prints out debugging info */
	void dump(int, int);
	
	/* use this after you have first created your widgets to set their positioning 
	   xd and yd have possible values of 1, 0, -1, I think these mean "left, middle, right; top, middle, bottom */
	void layout(int id, int xd, int yd);
	
	
	int  search(int, int, int);
	int  activate_widget(int, int, int);
	
	/* you only need to call this for parents, children will automatically be deleted by their parents */
	int  delete_widget(int);
	
	/*---------------------------------------------------------------------------*/
	
	/* call one a frame to update your widgets on the screen
	   You only need to call this for parents, children will automatically be painted by their parents */
	void paint(int);
	
	/* call "gui_pulse(gui_point(id, x, y), 1.2f);" whenever the mouse moves to make widgets pulse when the mouse goes over them */
	void pulse(int, float);
	
	/* call one a frame, passing on the value given to BaseGUI::update(int)
	   You only need to call this for parents, children will automatically be updated by their parents */
	void timer(int, float);
	
	/* mouse movement */	
	int  point(int, int x, int y);	
	
	/* joystick movement */
	int  stick(int, int xrel, int yrel);
	
	/* keyboard cursors */
	int cursor(int id, SDLKey key);
	
	/* mouse click */
	int  click();
	
	/* called if the game is paused */
	void blank();
	
	/* returns value of id's token */
	int  token(int) const;
	/* returns value of id's value */
	int  value(int) const;
	/* where id's value is being used as a bool, this toggles it */
	void toggle(int);
	
	/*---------------------------------------------------------------------------*/
	
	void tgl_paused();
	bool get_paused() const { return paused; }
	
private:
	int hot(int id);
	
	GLuint list(int x, int y, int w, int h, const float *c0, const float *c1);
	GLuint rect(int x, int y, int w, int h, int f, int r);
	int add_widget(int pd, int type);
	
	int pause(int);
	
	/*---------------------------------------------------------------------------*/
	
	void widget_up(int id);
	void harray_up(int id);
	void varray_up(int id);
	void hstack_up(int id);
	void vstack_up(int id);
	void paused_up(int id);
	void button_up(int id);
	
	void widget_dn(int id, int x, int y, int w, int h);
	void harray_dn(int id, int x, int y, int w, int h);
	void varray_dn(int id, int x, int y, int w, int h);
	void hstack_dn(int id, int x, int y, int w, int h);
	void vstack_dn(int id, int x, int y, int w, int h);
	void filler_dn(int id, int x, int y, int w, int h);
	void button_dn(int id, int x, int y, int w, int h);
	
	/*---------------------------------------------------------------------------*/
	
	void paint_rect(int id, int st);
	void paint_text(int id);;
	void paint_array(int id);
	void paint_image(int id);
	void paint_count(int id);
	void paint_clock(int id);
	void paint_label(int id);
	
	/*---------------------------------------------------------------------------*/

	int vert_test(int id, int jd);
	int horz_test(int id, int jd);
	
	int stick_L(int id, int dd);
	int stick_R(int id, int dd);
	int stick_D(int id, int dd);
	int stick_U(int id, int dd);
	
	/*---------------------------------------------------------------------------*/
	
	void config_push_persp(float, float, float);
	void config_push_ortho();
	void config_pop_matrix();
	void config_clear();
	
	/*---------------------------------------------------------------------------*/
	
	void set_paused();
	void clr_paused();
	
	/*---------------------------------------------------------------------------*/
	
	Widget widgets[MAXWIDGETS];
	int           active;
	int           radius;
	TTF_Font     *font[3];
	
	GLuint digit_text[3][11];
	GLuint digit_list[3][11];
	int    digit_w[3][11];
	int    digit_h[3][11];
	
	int pause_id;
	bool paused;
};


#endif

/* EOF */
