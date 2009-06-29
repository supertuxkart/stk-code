//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#ifndef HEADER_SKIN_HPP
#define HEADER_SKIN_HPP

#include "irrlicht.h"
#include "utils/ptr_vector.hpp"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace GUIEngine
{
    
    /**
      * In order to avoid calculating render information every frame, it's stored in a
      * SkinWidgetContainer for each widget (or each widget part if it requires many)
      */
    class SkinWidgetContainer
    {
    public:
        int x, y, w, h;
        
        bool dest_areas_inited;
        bool dest_areas_yflip_inited;
        int dest_x, dest_y, dest_x2, dest_y2;
        
        // see comments in Skin::drawBoxFromStretchableTexture for explaination of
        // what these are
        core::rect<s32> dest_area_left;
        core::rect<s32> dest_area_center;
        core::rect<s32> dest_area_right;
        
        core::rect<s32> dest_area_top;
        core::rect<s32> dest_area_bottom;
        
        core::rect<s32> dest_area_top_left;
        core::rect<s32> dest_area_top_right;
        core::rect<s32> dest_area_bottom_left;
        core::rect<s32> dest_area_bottom_right;
        
        // y flip
        core::rect<s32> dest_area_left_yflip;
        core::rect<s32> dest_area_center_yflip;
        core::rect<s32> dest_area_right_yflip;
        
        core::rect<s32> dest_area_top_yflip;
        core::rect<s32> dest_area_bottom_yflip;
        
        core::rect<s32> dest_area_top_left_yflip;
        core::rect<s32> dest_area_top_right_yflip;
        core::rect<s32> dest_area_bottom_left_yflip;
        core::rect<s32> dest_area_bottom_right_yflip;   
        
        SkinWidgetContainer()
        {
            dest_areas_inited = false;
            dest_areas_yflip_inited = false;
            x = -1;
            y = -1;
            w = -1;
            h = -1;
        }
    };
    
    class Widget;
    
    /** class containing render params for the 'drawBoxFromStretchableTexture' function */
    class BoxRenderParams
    {
        ITexture* image;
        bool y_flip_set;

    public:
        ITexture* getImage() { return image; }
        
        int left_border, right_border, top_border, bottom_border;
        bool preserve_h_aspect_ratios;
        float hborder_out_portion, vborder_out_portion;
        
        // this parameter is a bit special since it's the only one that can change at runtime
        bool vertical_flip;
        
        /** bitmap containing which areas to render */
        int areas;
        // possible values in areas
        static const int BODY = 1;
        static const int LEFT = 2;
        static const int RIGHT = 4;
        static const int TOP = 8;
        static const int BOTTOM = 16;
        
        core::rect<s32> source_area_left;
        core::rect<s32> source_area_center;
        core::rect<s32> source_area_right;
        
        core::rect<s32> source_area_top;
        core::rect<s32> source_area_bottom;
        
        core::rect<s32> source_area_top_left;
        core::rect<s32> source_area_top_right;
        core::rect<s32> source_area_bottom_left;
        core::rect<s32> source_area_bottom_right; 
        
        
        // y-flipped coords
        core::rect<s32> source_area_left_yflip;
        core::rect<s32> source_area_center_yflip;
        core::rect<s32> source_area_right_yflip;
        
        core::rect<s32> source_area_top_yflip;
        core::rect<s32> source_area_bottom_yflip;
        
        core::rect<s32> source_area_top_left_yflip;
        core::rect<s32> source_area_top_right_yflip;
        core::rect<s32> source_area_bottom_left_yflip;
        core::rect<s32> source_area_bottom_right_yflip; 
        
        BoxRenderParams();
        void setTexture(ITexture* image);
        void calculateYFlipIfNeeded();
    };
    
    class Skin : public IGUISkin
    {
        IGUISkin* m_fallback_skin;
        

        ITexture* bg_image;
        

        void drawBoxFromStretchableTexture(SkinWidgetContainer* w, const core::rect< s32 > &dest, BoxRenderParams& params);
        
        // my utility methods, to work around irrlicht's very Windows-95-like-look-enforcing skin system
        void process3DPane(IGUIElement *element, const core::rect< s32 > &rect, const bool pressed);
        void drawButton(Widget* w, const core::rect< s32 > &rect, const bool pressed, const bool focused);
        void drawRibbon(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused);
        void drawRibbonChild(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused);
        void drawSpinnerChild(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused);
        void drawSpinnerBody(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused);
        void drawGauge(const core::rect< s32 > &rect, Widget* widget, bool focused);
        void drawGaugeFill(const core::rect< s32 > &rect, Widget* widget, bool focused);
        void drawCheckBox(const core::rect< s32 > &rect, Widget* widget, bool focused);
        void drawList(const core::rect< s32 > &rect, Widget* widget, bool focused);
        void drawListSelection(const core::rect< s32 > &rect, Widget* widget, bool focused);

    public:
        Skin(IGUISkin* fallback_skin);
        ~Skin();

        void renderSections(ptr_vector<Widget>* within_vector=NULL);
        void drawBgImage();
        
        // irrlicht's callbacks
        virtual void 	draw2DRectangle (IGUIElement *element, const video::SColor &color, const core::rect< s32 > &pos, const core::rect< s32 > *clip);
        virtual void 	draw3DButtonPanePressed (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);
        virtual void 	draw3DButtonPaneStandard (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);
        virtual void 	draw3DMenuPane (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);
        virtual void 	draw3DSunkenPane (IGUIElement *element, video::SColor bgcolor, bool flat, bool fillBackGround, const core::rect< s32 > &rect, const core::rect< s32 > *clip);
        virtual void 	draw3DTabBody (IGUIElement *element, bool border, bool background, const core::rect< s32 > &rect, const core::rect< s32 > *clip, s32 tabHeight=-1, gui::EGUI_ALIGNMENT alignment=EGUIA_UPPERLEFT);
        virtual void 	draw3DTabButton (IGUIElement *element, bool active, const core::rect< s32 > &rect, const core::rect< s32 > *clip, gui::EGUI_ALIGNMENT alignment=EGUIA_UPPERLEFT);
        virtual void 	draw3DToolBar (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);
        virtual core::rect< s32 > 	draw3DWindowBackground (IGUIElement *element, bool drawTitleBar, video::SColor titleBarColor, const core::rect< s32 > &rect, const core::rect< s32 > *clip);
        virtual void 	drawIcon (IGUIElement *element, EGUI_DEFAULT_ICON icon, const core::position2di position, u32 starttime, u32 currenttime, bool loop=false, const core::rect< s32 > *clip=NULL);
        virtual video::SColor 	getColor (EGUI_DEFAULT_COLOR color) const;
        virtual const wchar_t * 	getDefaultText (EGUI_DEFAULT_TEXT text) const;
        virtual IGUIFont * 	getFont (EGUI_DEFAULT_FONT which=EGDF_DEFAULT) const ;
        virtual u32 	getIcon (EGUI_DEFAULT_ICON icon) const ;
        virtual s32 	getSize (EGUI_DEFAULT_SIZE size) const ;
        virtual IGUISpriteBank * 	getSpriteBank () const ;
        //virtual EGUI_SKIN_TYPE 	getType () const;
        virtual void 	setColor (EGUI_DEFAULT_COLOR which, video::SColor newColor);
        virtual void 	setDefaultText (EGUI_DEFAULT_TEXT which, const wchar_t *newText);
        virtual void 	setFont (IGUIFont *font, EGUI_DEFAULT_FONT which=EGDF_DEFAULT);
        virtual void 	setIcon (EGUI_DEFAULT_ICON icon, u32 index);
        virtual void 	setSize (EGUI_DEFAULT_SIZE which, s32 size);
        virtual void 	setSpriteBank (IGUISpriteBank *bank);
        
    };
}
#endif
