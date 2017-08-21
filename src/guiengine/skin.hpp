//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include <string>

#include <rect.h>
#include <SColor.h>
#include <vector2d.h>
#include <dimension2d.h>
#include <IGUISkin.h>
namespace irr
{
    namespace video { class ITexture;    }
    namespace gui   { class IGUIElement; class IGUIFont; class IGUISpriteBank; }
}

using namespace irr;


#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

/**
 \page skin Overview of GUI skin

 The GUIEngine::Skin is the one handling skinning. It loads images and their
 sizing from a XML file. Since the irrLicht way of handling skin is quite
 "boxy" and results in games looking like Window 95, this class overrides it
 very much; in pretty much all callbacks, rather drawing plainly what irrLicht
 asks it to draw, it first checks which widget we're asked to render and
 redirects the call to a more specific method.

 Furthermore, since irrLicht widgets were quite basic, a few widgets were
 created by combining several irrLicht widgets (e.g. 2 buttons and a label in
 a box make a spinner). Because of this, some jumping through hoops is
 performed (we get a callback for each of these sub-widgets, but want to draw
 the whole thing as a single block)

 There are two types of images : some will be simply stretched as a whole,
 others will have non-stretchable borders (you cannot choose which one you
 must use, it's hardcoded for each element type; though, as you will see
 below, for all "advanced stretching" images you can easily fake "simple
 stretch")

 \section Describing a skin in a XML file

 All elements will have at least 2 properties :
 \li \c type="X" : sets what you're skinning with this entry
 \li \c image="skinDirectory/imageName.png" : sets which image is used for
      this element

 For more information, I highly recommend simply looking at existing skins,
 they will show the format of the XML file describing a skin quite well.

 \section states Widget States

 Most elements also support states :
 \li \c state="neutral"
 \li \c state="focused"
 \li \c state="down"

 You can thus give different looks for different states.  Not all widgets
 support all states, see entries and comments below to know what's
 supported. Note that checkboxes are an exception and have the following
  styles :
 \li \lc "neutral+unchecked"
 \li \lc "neutral+checked"
 \li \lc "focused+unchecked"
 \li \lc "focused+checked"

 \section stretch Advanced stretching
 "Advanced stretching" images are split this way :

 \code
 +----+--------------------+----+
 |    |                    |    |
 +----+--------------------+----+
 |    |                    |    |
 |    |                    |    |
 |    |                    |    |
 +----+--------------------+----+
 |    |                    |    |
 +----+--------------------+----+
 \endcode

 The center border will be stretched in all directions. The 4 corners will not
 stretch at all. Horizontal borders will stretch horizontally, vertical
 borders will stretch vertically. Use properties left_border="X"
 right_border="X" top_border="X" bottom_border="X" to specify the size of each
 border in pixels (setting all borders to '0' makes the whole image scaled).

 In some cases, you may not want vertical stretching to occur (like if the left
 and right sides of the image must not be stretched vertically, e.g. for the
 spinner). In this case, pass parameter preserve_h_aspect_ratios="true" to
 make the left and right areas stretch by keeping their aspect ratio.

 Some components may fill the full inner area with stuff; others will only take
 a smaller area at the center. To adjust for this, there are properties
 "hborder_out_portion" and "vborder_out_portion" that take a float from 0 to 1,
 representing the percentage of each border that goes out of the widget's area
 (this might include stuff like shadows, etc.). The 'h' one is for horizontal
 borders, the 'v' one is for vertical borders.

 Finnally : the image is split, as shown above, into 9 areas. In some cases,
 you may not want all areas to be rendered. Then you can pass parameter
 areas="body+left+right+top+bottom" and explicitely specify which parts you
 want to see. The 4 corner areas are only visible when the border that
 intersect at this corner are enabled.

  */

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
    /**
      * In order to avoid calculating render information every frame, it's
      * stored in a SkinWidgetContainer for each widget (or each widget part
      * if it requires many)
      * \ingroup guiengine
      */
    class SkinWidgetContainer
    {
    public:
        int m_skin_x, m_skin_y, m_skin_w, m_skin_h;

        bool m_skin_dest_areas_inited;
        bool m_skin_dest_areas_yflip_inited;
        int m_skin_dest_x, m_skin_dest_y, m_skin_dest_x2, m_skin_dest_y2;

        // see comments in Skin::drawBoxFromStretchableTexture for
        // explaination of what these are
        core::rect<s32> m_skin_dest_area_left;
        core::rect<s32> m_skin_dest_area_center;
        core::rect<s32> m_skin_dest_area_right;

        core::rect<s32> m_skin_dest_area_top;
        core::rect<s32> m_skin_dest_area_bottom;

        core::rect<s32> m_skin_dest_area_top_left;
        core::rect<s32> m_skin_dest_area_top_right;
        core::rect<s32> m_skin_dest_area_bottom_left;
        core::rect<s32> m_skin_dest_area_bottom_right;

        // y flip
        core::rect<s32> m_skin_dest_area_left_yflip;
        core::rect<s32> m_skin_dest_area_center_yflip;
        core::rect<s32> m_skin_dest_area_right_yflip;

        core::rect<s32> m_skin_dest_area_top_yflip;
        core::rect<s32> m_skin_dest_area_bottom_yflip;

        core::rect<s32> m_skin_dest_area_top_left_yflip;
        core::rect<s32> m_skin_dest_area_top_right_yflip;
        core::rect<s32> m_skin_dest_area_bottom_left_yflip;
        core::rect<s32> m_skin_dest_area_bottom_right_yflip;

        short m_skin_r, m_skin_g, m_skin_b;

        SkinWidgetContainer()
        {
            m_skin_dest_areas_inited = false;
            m_skin_dest_areas_yflip_inited = false;
            m_skin_x = -1;
            m_skin_y = -1;
            m_skin_w = -1;
            m_skin_h = -1;
            m_skin_r = -1;
            m_skin_g = -1;
            m_skin_b = -1;
        }   // SkinWidgetContainer
    };   // class SkinWidgetContainer

    // ========================================================================
    class Widget;

    /**
      * \brief class containing render params for the
      * 'drawBoxFromStretchableTexture' function see \ref skin for more
      * information about skinning in STK
      * \ingroup guiengine
      */
    class BoxRenderParams
    {
        video::ITexture* m_image;
        bool             m_y_flip_set;

    public:
        int m_left_border, m_right_border, m_top_border, m_bottom_border;
        bool m_preserve_h_aspect_ratios;
        float m_hborder_out_portion, m_vborder_out_portion;

        // this parameter is a bit special since it's the only one that can
        // change at runtime
        bool m_vertical_flip;

        /** bitmap containing which areas to render */
        int areas;
        // possible values in areas
        static const int BODY = 1;
        static const int LEFT = 2;
        static const int RIGHT = 4;
        static const int TOP = 8;
        static const int BOTTOM = 16;

        core::rect<s32> m_source_area_left;
        core::rect<s32> m_source_area_center;
        core::rect<s32> m_source_area_right;

        core::rect<s32> m_source_area_top;
        core::rect<s32> m_source_area_bottom;

        core::rect<s32> m_source_area_top_left;
        core::rect<s32> m_source_area_top_right;
        core::rect<s32> m_source_area_bottom_left;
        core::rect<s32> m_source_area_bottom_right;


        // y-flipped coords
        core::rect<s32> m_source_area_left_yflip;
        core::rect<s32> m_source_area_center_yflip;
        core::rect<s32> m_source_area_right_yflip;

        core::rect<s32> m_source_area_top_yflip;
        core::rect<s32> m_source_area_bottom_yflip;

        core::rect<s32> m_source_area_top_left_yflip;
        core::rect<s32> m_source_area_top_right_yflip;
        core::rect<s32> m_source_area_bottom_left_yflip;
        core::rect<s32> m_source_area_bottom_right_yflip;

        BoxRenderParams();
        void setTexture(video::ITexture* image);
        void calculateYFlipIfNeeded();
        // --------------------------------------------------------------------
        /** Returns the image for this BoxRenderParams. */
        video::ITexture* getImage() { return m_image; }
    };   // BoxRenderParams

    // ========================================================================
    /**
      * \brief Object used to render the GUI widgets
      * see \ref skin for more information about skinning in STK
      * \ingroup guiengine
      */
    class Skin : public gui::IGUISkin
    {
        gui::IGUISkin* m_fallback_skin;


        video::ITexture* bg_image;
        std::vector<Widget*> m_tooltips;
        std::vector<bool> m_tooltip_at_mouse;

        LEAK_CHECK()

        void drawBoxFromStretchableTexture(SkinWidgetContainer* w,
                                         const core::rect< s32 > &dest,
                                         BoxRenderParams& params,
                                         bool deactivated=false,
                                         const core::rect<s32>* clipRect=NULL);
    private:
        // my utility methods, to work around irrlicht's very
        // Windows-95-like-look-enforcing skin system
        void process3DPane(gui::IGUIElement *element,
                           const core::rect< s32 > &rect, const bool pressed);
        void drawButton(Widget* w, const core::rect< s32 > &rect,
                        const bool pressed, const bool focused);
        void drawProgress(Widget* w, const core::rect< s32 > &rect,
                          const bool pressed, const bool focused);
        void drawRatingBar(Widget* w, const core::rect< s32 > &rect,
                          const bool pressed, const bool focused);
        void drawRibbon(const core::rect< s32 > &rect, Widget* widget,
                        const bool pressed, bool focused);
        void drawRibbonChild(const core::rect< s32 > &rect, Widget* widget,
                             const bool pressed, bool focused);
        void drawSpinnerChild(const core::rect< s32 > &rect, Widget* widget,
                               const bool pressed, bool focused);
        void drawSpinnerBody(const core::rect< s32 > &rect, Widget* widget,
                             const bool pressed, bool focused);
        void drawGauge(const core::rect< s32 > &rect, Widget* widget,
                       bool focused);
        void drawGaugeFill(const core::rect< s32 > &rect, Widget* widget,
                           bool focused);
        void drawCheckBox(const core::rect< s32 > &rect, Widget* widget,
                          bool focused);
        void drawList(const core::rect< s32 > &rect, Widget* widget,
                      bool focused);
        void drawListHeader(const core::rect< s32 > &rect, Widget* widget);
        void drawListSelection(const core::rect< s32 > &rect, Widget* widget,
                               bool focused, const core::rect< s32 > *clip);
        void drawIconButton(const core::rect< s32 > &rect, Widget* widget,
                            const bool pressed, bool focused);
        void drawScrollbarBackground(const core::rect< s32 > &rect);
        void drawScrollbarThumb(const core::rect< s32 > &rect);
        void drawScrollbarButton(const core::rect< s32 > &rect,
                                 const bool pressed, const bool bottomArrow);

        void drawTooltip(Widget* widget, bool atMouse);


    public:

        // dirty way to have dialogs that zoom in
        bool m_dialog;
        float m_dialog_size;
        /**
          * \brief load a skin from the file specified in the user configuration file
          * \throw std::runtime_error if file cannot be read
          */
        Skin(gui::IGUISkin* fallback_skin);

        ~Skin();

        static video::SColor getColor(const std::string &name);
        void renderSections(PtrVector<Widget>* within_vector=NULL);
        void drawBgImage();
        void drawBGFadeColor();
        void drawBadgeOn(const Widget* widget, const core::rect<s32>& rect);
        void drawProgressBarInScreen(SkinWidgetContainer* swc,
                                     const core::rect< s32 > &rect,
                                     int progress, bool deactivated = false);

        // irrlicht's callbacks
        virtual void draw2DRectangle (gui::IGUIElement *element,
                                      const video::SColor &color,
                                      const core::rect< s32 > &pos,
                                      const core::rect< s32 > *clip);
        virtual void draw3DButtonPanePressed(gui::IGUIElement *element,
                                             const core::rect< s32 > &rect,
                                             const core::rect< s32 > *clip);
        virtual void draw3DButtonPaneStandard(gui::IGUIElement *element,
                                   const core::rect< s32 > &rect,
                                   const core::rect< s32 > *clip);
        virtual void draw3DMenuPane (gui::IGUIElement *element,
                                     const core::rect< s32 > &rect,
                                     const core::rect< s32 > *clip);
        virtual void draw3DSunkenPane (gui::IGUIElement *element,
                                       video::SColor bgcolor,
                                       bool flat, bool fillBackGround,
                                       const core::rect< s32 > &rect,
                                       const core::rect< s32 > *clip);
        virtual void draw3DTabBody (gui::IGUIElement *element, bool border,
                                    bool background,
                                    const core::rect< s32 > &rect,
                                    const core::rect< s32 > *clip,
                                    s32 tabHeight=-1,
                                    gui::EGUI_ALIGNMENT alignment=
                                                         gui::EGUIA_UPPERLEFT);
        virtual void draw3DTabButton (gui::IGUIElement *element,
                                      bool active,
                                      const core::rect< s32 > &rect,
                                      const core::rect< s32 > *clip,
                                      gui::EGUI_ALIGNMENT alignment=
                                                         gui::EGUIA_UPPERLEFT);
        virtual void draw3DToolBar (gui::IGUIElement *element,
                                    const core::rect< s32 > &rect,
                                    const core::rect< s32 > *clip);
        virtual core::rect< s32 >
                        draw3DWindowBackground(gui::IGUIElement *element,
                                           bool drawTitleBar,
                                           video::SColor titleBarColor,
                                           const core::rect< s32 > &rect,
                                           const core::rect< s32 > *clip,
                                           core::rect<s32>* checkClientArea=0);

        virtual void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
            const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
            const video::SColor* const colors, bool useAlphaChannelOfTexture);

        virtual void drawIcon (gui::IGUIElement *element,
                               gui::EGUI_DEFAULT_ICON icon,
                               const core::position2di position,
                               u32 starttime, u32 currenttime,
                               bool loop=false,
                               const core::rect< s32 > *clip=NULL);
        virtual video::SColor  getColor (gui::EGUI_DEFAULT_COLOR color) const;
        virtual const wchar_t*
                             getDefaultText(gui::EGUI_DEFAULT_TEXT text) const;
        virtual gui::IGUIFont* getFont(gui::EGUI_DEFAULT_FONT which=
                                                     gui::EGDF_DEFAULT) const;
        virtual u32  getIcon (gui::EGUI_DEFAULT_ICON icon) const;
        virtual s32  getSize (gui::EGUI_DEFAULT_SIZE size) const;
        const BoxRenderParams& getBoxRenderParams(const std::string &type);
        virtual gui::IGUISpriteBank *  getSpriteBank () const;
        virtual void setColor (gui::EGUI_DEFAULT_COLOR which,
                                  video::SColor newColor);
        virtual void setDefaultText (gui::EGUI_DEFAULT_TEXT which,
                                        const wchar_t* newText);
        virtual void setFont (gui::IGUIFont *font,
                              gui::EGUI_DEFAULT_FONT which=gui::EGDF_DEFAULT);
        virtual void setIcon (gui::EGUI_DEFAULT_ICON icon, u32 index);
        virtual void setSize (gui::EGUI_DEFAULT_SIZE which, s32 size);
        virtual void setSpriteBank (gui::IGUISpriteBank *bank);

        void drawTooltips();
        void drawMessage(SkinWidgetContainer* w, const core::recti &dest,
                         const std::string &type);

        video::ITexture* getImage(const char* name);

        gui::IGUISkin* getFallbackSkin() { return m_fallback_skin; }


    };   // Skin
}   // guiengine
#endif
