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

#include "guiengine/skin.hpp"

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "config/user_config.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/central_settings.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/widgets.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"

using namespace GUIEngine;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

const bool ID_DEBUG = false;

/**
 * Small utility to read config file info from a XML file.
 */
namespace SkinConfig
{
    static std::map<std::string, BoxRenderParams> m_render_params;
    static std::map<std::string, SColor> m_colors;

    static void parseElement(const XMLNode* node)
    {
        std::string type;
        std::string state = "neutral";
        std::string image;
        int leftborder = 0, rightborder=0, topborder=0, bottomborder=0;
        float hborder_out_portion = 0.5f, vborder_out_portion = 1.0f;
        bool preserve_h_aspect_ratios = false;
        std::string areas;

        if (node->get("type", &type) == 0)
        {
            Log::error("skin", "All elements must have a type\n");
            return;
        }
        node->get("state", &state);

        if (node->get("image", &image) == 0)
        {
            Log::error("skin", "All elements must have an image\n");
            return;
        }

        node->get("left_border", &leftborder);
        node->get("right_border", &rightborder);
        node->get("top_border", &topborder);
        node->get("bottom_border", &bottomborder);

        node->get("hborder_out_portion", &hborder_out_portion);
        node->get("vborder_out_portion", &vborder_out_portion);

        node->get("preserve_h_aspect_ratios", &preserve_h_aspect_ratios);

        node->get("areas", &areas);


        BoxRenderParams new_param;
        new_param.m_left_border = leftborder;
        new_param.m_right_border = rightborder;
        new_param.m_top_border = topborder;
        new_param.m_bottom_border = bottomborder;
        new_param.m_hborder_out_portion = hborder_out_portion;
        new_param.m_vborder_out_portion = vborder_out_portion;
        new_param.m_preserve_h_aspect_ratios = preserve_h_aspect_ratios;

        // call last since it calculates coords considering all other
        // parameters
        new_param.setTexture( irr_driver->getTexture(FileManager::SKIN, image));

        if (areas.size() > 0)
        {
            new_param.areas = 0;
            if(areas.find("body")   != std::string::npos)
                new_param.areas |= BoxRenderParams::BODY;
            if(areas.find("top")    != std::string::npos)
                new_param.areas |= BoxRenderParams::TOP;
            if(areas.find("bottom") != std::string::npos)
                new_param.areas |= BoxRenderParams::BOTTOM;
            if(areas.find("left")   != std::string::npos)
                new_param.areas |= BoxRenderParams::LEFT;
            if(areas.find("right")  != std::string::npos)
                new_param.areas |= BoxRenderParams::RIGHT;
        }

        m_render_params[type+"::"+state] = new_param;
    }   // parseElement

    // ------------------------------------------------------------------------
    static void parseColor(const XMLNode* node)
    {
        std::string type;
        std::string state = "neutral";
        int r = 0, g = 0, b = 0;

        if(node->get("type", &type) == 0)
        {
            Log::error("skin", "All elements must have a type\n");
            return;
        }
        node->get("state", &state);

        node->get("r", &r);
        node->get("g", &g);
        node->get("b", &b);

        int a = 255;
        node->get("a", &a);

        SColor color = SColor(a, r, g, b);

        m_colors[type+"::"+state] = color;
    }   // parseColor
    // ------------------------------------------------------------------------
    /**
      * \brief loads skin information from a STK skin file
      * \throw std::runtime_error if file cannot be read
      */
    static void loadFromFile(std::string file)
    {
        XMLNode* root = file_manager->createXMLTree(file);
        if(!root)
        {
            Log::error("skin", "Could not read XML file '%s'.",
                       file.c_str());
            throw std::runtime_error("Invalid skin file");
        }

        const int amount = root->getNumNodes();
        for (int i=0; i<amount; i++)
        {
            const XMLNode* node = root->getNode(i);

            if (node->getName() == "element")
            {
                parseElement(node);
            }
            else if (node->getName() == "color")
            {
                parseColor(node);
            }
            else
            {
                Log::error("skin", "Unknown node in XML file '%s'.",
                           node->getName().c_str());
            }
        }// nend for

        delete root;
    }   // loadFromFile
};   // SkinConfig

// ============================================================================
#if 0
#pragma mark -
#endif

/** load default values */
BoxRenderParams::BoxRenderParams()
{
    m_left_border = 0;
    m_right_border = 0;
    m_top_border = 0;
    m_bottom_border = 0;
    m_preserve_h_aspect_ratios = false;

    m_hborder_out_portion = 0.5;
    m_vborder_out_portion = 1.0;

    areas = BODY | LEFT | RIGHT | TOP | BOTTOM;
    m_vertical_flip = false;
    m_y_flip_set = false;
}   // BoxRenderParams

// ----------------------------------------------------------------------------
void BoxRenderParams::setTexture(ITexture* image)
{
    if (image == NULL)
    {
        Log::error("skin", "/!\\ WARNING: missing image in skin\n");
        return;
    }

    m_image = image;
    /*
     The source texture is split this way to allow for a stretchable center
     and borders that don't stretch :

     +----+--------------------+----+
     |    |                    |    |
     +----a--------------------b----+ <-- top_border
     |    |                    |    |
     |    |                    |    |
     +----c--------------------d----+ <-- height - bottom-border
     |    |                    |    |
     +----+--------------------+----+
     */

    const int texture_w = image->getSize().Width;
    const int texture_h = image->getSize().Height;

    const int ax = m_left_border;
    const int ay = m_top_border;
    const int bx = texture_w - m_right_border;
    const int by = m_top_border;
    const int cx = m_left_border;
    const int cy = texture_h - m_bottom_border;
    const int dx = texture_w - m_right_border;
    const int dy = texture_h - m_bottom_border;

    m_source_area_left         = core::recti(0, ay, cx, cy);
    m_source_area_center       = core::recti(ax, ay, dx, dy);
    m_source_area_right        = core::recti(bx, m_top_border, texture_w, dy);

    m_source_area_top          = core::recti(ax, 0, bx, by);
    m_source_area_bottom       = core::recti(cx, cy, dx, texture_h);

    m_source_area_top_left     = core::recti(0, 0, ax, ay);
    m_source_area_top_right    = core::recti(bx, 0, texture_w, m_top_border);
    m_source_area_bottom_left  = core::recti(0, cy, cx, texture_h);
    m_source_area_bottom_right = core::recti(dx, dy, texture_w, texture_h);
}   // setTexture

// ----------------------------------------------------------------------------
void BoxRenderParams::calculateYFlipIfNeeded()
{
    if (m_y_flip_set) return;

#define FLIP_Y( X ) {     const int y1 = X.UpperLeftCorner.Y; \
const int y2 = X.LowerRightCorner.Y; \
X##_yflip = X; \
X##_yflip.UpperLeftCorner.Y =  y2;\
X##_yflip.LowerRightCorner.Y =  y1;}

    FLIP_Y(m_source_area_left)
    FLIP_Y(m_source_area_center)
    FLIP_Y(m_source_area_right)

    FLIP_Y(m_source_area_top)
    FLIP_Y(m_source_area_bottom)

    FLIP_Y(m_source_area_top_left)
    FLIP_Y(m_source_area_top_right)
    FLIP_Y(m_source_area_bottom_left)
    FLIP_Y(m_source_area_bottom_right)
#undef FLIP_Y


    m_y_flip_set = true;
}   // calculateYFlipIfNeeded

// ----------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

Skin::Skin(IGUISkin* fallback_skin)
{
    std::string skin_name = file_manager->getAsset(FileManager::SKIN,
                                                   UserConfigParams::m_skin_file);

    try
    {
        SkinConfig::loadFromFile( skin_name );
    }
    catch (std::runtime_error& e)
    {
        (void)e;   // avoid compiler warning
        // couldn't load skin. Try to revert to default
        UserConfigParams::m_skin_file.revertToDefaults();

        skin_name = file_manager->getAsset(FileManager::SKIN,
                                           UserConfigParams::m_skin_file);
        SkinConfig::loadFromFile( skin_name );
    }

    bg_image = NULL;

    m_fallback_skin = fallback_skin;
    m_fallback_skin->grab();
    assert(fallback_skin != NULL);

    m_dialog = false;
    m_dialog_size = 0.0f;
}   // Skin

// ----------------------------------------------------------------------------
Skin::~Skin()
{
    m_fallback_skin->drop();
}   // ~Skin

// ----------------------------------------------------------------------------
void Skin::drawBgImage()
{
#ifndef SERVER_ONLY
    // ---- background image
    // on one end, making these static is not too clean.
    // on another end, these variables are really only used locally,
    // and making them static avoids doing the same stupid computations
    // every frame
    static core::recti dest;
    static core::recti source_area;

    if(bg_image == NULL)
    {
        int texture_w, texture_h;
        bg_image =
            SkinConfig::m_render_params["background::neutral"].getImage();
        assert(bg_image != NULL);
        texture_w = bg_image->getSize().Width;
        texture_h = bg_image->getSize().Height;

        source_area = core::recti(0, 0, texture_w, texture_h);

        core::dimension2d<u32> frame_size = irr_driver->getActualScreenSize();
        const int screen_w = frame_size.Width;
        const int screen_h = frame_size.Height;

        // stretch image vertically to fit
        float ratio = (float)screen_h / texture_h;

        // check that with the vertical stretching, it still fits horizontally
        while(texture_w*ratio < screen_w) ratio += 0.1f;

        texture_w = (int)(texture_w*ratio);
        texture_h = (int)(texture_h*ratio);

        const int clipped_x_space = (texture_w - screen_w);

        dest = core::recti(-clipped_x_space/2, 0,
                               screen_w+clipped_x_space/2, texture_h);
    }

    irr_driver->getVideoDriver()->enableMaterial2D();
    draw2DImage(bg_image, dest, source_area,
                                        /* no clipping */0, /*color*/ 0,
                                        /*alpha*/false);
    irr_driver->getVideoDriver()->enableMaterial2D(false);
#endif
}   // drawBgImage

// ----------------------------------------------------------------------------
/** Returns the BoxRenderParams data structure for a given type.
 *  \param type The type name of the box render param to get.
 */
const BoxRenderParams& Skin::getBoxRenderParams(const std::string &type)
{
    return SkinConfig::m_render_params[type];
}   // getBoxRenderParams

// ----------------------------------------------------------------------------
/** Draws a background box for an in-game notification message. Example would
 *  be an achievement, or friends comming online.
 *  \param w The SkinWidgetContainer for the outline.
 *  \param dest The destination rectangle to use.
 *  \param type The type of the message (achievement or friend).
 */
void Skin::drawMessage(SkinWidgetContainer* w, const core::recti &dest,
                       const std::string &type)
{
    drawBoxFromStretchableTexture(w, dest, SkinConfig::m_render_params[type]);
}   // drawMessage

// ----------------------------------------------------------------------------
void Skin::drawBoxFromStretchableTexture(SkinWidgetContainer* w,
                                         const core::recti &dest,
                                         BoxRenderParams& params,
                                         bool deactivated,
                                         const core::recti* clipRect)
{
#ifndef SERVER_ONLY
    // check if widget moved. if so, recalculate coords
    if (w->m_skin_x != dest.UpperLeftCorner.X ||
        w->m_skin_y != dest.UpperLeftCorner.Y ||
        w->m_skin_w != dest.getWidth()        ||
        w->m_skin_h != dest.getHeight()            )
    {
        w->m_skin_dest_areas_inited = false;
        w->m_skin_dest_areas_yflip_inited = false;
        w->m_skin_x = dest.UpperLeftCorner.X;
        w->m_skin_y = dest.UpperLeftCorner.Y;
        w->m_skin_w = dest.getWidth();
        w->m_skin_h = dest.getHeight();
    }

    const ITexture* source              = params.getImage();
    const int left_border               = params.m_left_border;
    const int right_border              = params.m_right_border;
    const int top_border                = params.m_top_border;
    const int bottom_border             = params.m_bottom_border;
    const bool preserve_h_aspect_ratios = params.m_preserve_h_aspect_ratios;
    const float hborder_out_portion     = params.m_hborder_out_portion;
    const float vborder_out_portion     = params.m_vborder_out_portion;
    const bool vertical_flip            = params.m_vertical_flip;
    int areas = params.areas;

    const int texture_h = source->getSize().Height;

    /*
     The dest area is split this way. Borders can go a bit beyond the given
     area so components inside don't go over the borders
     (how much it exceeds horizontally is specified in 'hborder_out_portion'.
     vertically is always the totality)

     a----b--------------------c----+
     |    |                    |    |
     d----e--------------------f----g  <-- top_border
     |    |                    |    |
     |    |                    |    |
     |    |                    |    |
     h----i--------------------j----k  <-- height - bottom-border
     |    |                    |    |
     +----l--------------------m----n
     */

    if (!w->m_skin_dest_areas_inited)
    {
        w->m_skin_dest_x  = dest.UpperLeftCorner.X;
        w->m_skin_dest_y  = dest.UpperLeftCorner.Y;
        w->m_skin_dest_x2 = dest.LowerRightCorner.X;
        w->m_skin_dest_y2 = dest.LowerRightCorner.Y;

        const float yscale =
            (float)(w->m_skin_dest_y2 - w->m_skin_dest_y)/texture_h;

        int dest_left_border, dest_right_border;

        // scale and keep aspect ratio
        if (preserve_h_aspect_ratios)
        {
            dest_left_border   = (int)(  left_border
                                       * (w->m_skin_dest_y2 - w->m_skin_dest_y)
                                       / texture_h);
            dest_right_border  = (int)(right_border
                                       * (w->m_skin_dest_y2 - w->m_skin_dest_y)
                                       / texture_h);
        }
        else
        {
            dest_left_border   =
                (int)(left_border  *std::min<float>(yscale, 1.0));
            dest_right_border  =
                (int)(right_border *std::min<float>(yscale, 1.0));
        }
        int dest_top_border    =
            (int)(top_border   *std::min<float>(yscale, 1.0));
        int dest_bottom_border =
            (int)(bottom_border*std::min<float>(yscale, 1.0));


        const float hborder_in_portion = 1 - hborder_out_portion;
        const float vborder_in_portion = 1 - vborder_out_portion;

        const int ax = (int)(w->m_skin_dest_x
                             - dest_left_border * hborder_out_portion);
        const int ay = (int)(w->m_skin_dest_y
                             - dest_top_border  * vborder_out_portion);

        const int bx = (int)(w->m_skin_dest_x
                             + dest_left_border*hborder_in_portion);
        const int by = ay;

        const int cx = (int)(w->m_skin_dest_x2
                             - dest_right_border*hborder_in_portion);
        const int cy = ay;

        const int dx = ax;
        const int dy = (int)(w->m_skin_dest_y
                             + dest_top_border*vborder_in_portion);

        const int ex = bx;
        const int ey = dy;

        const int fx = cx;
        const int fy = dy;

        const int gx = (int)(w->m_skin_dest_x2
                             + dest_right_border*hborder_out_portion);
        const int gy = dy;

        const int hx = ax;
        const int hy = (int)(w->m_skin_dest_y2
                             - dest_bottom_border*vborder_in_portion);

        const int ix = bx;
        const int iy = hy;

        const int jx = cx;
        const int jy = hy;

        const int kx = gx;
        const int ky = hy;

        const int lx = bx;
        const int ly = (int)(w->m_skin_dest_y2
                            + dest_bottom_border*vborder_out_portion);

        const int mx = cx;
        const int my = ly;

        const int nx = gx;
        const int ny = ly;

        w->m_skin_dest_area_left         = core::recti(dx, dy, ix, iy);
        w->m_skin_dest_area_center       = core::recti(ex, ey, jx, jy);
        w->m_skin_dest_area_right        = core::recti(fx, fy, kx, ky);

        w->m_skin_dest_area_top          = core::recti(bx, by, fx, fy);
        w->m_skin_dest_area_bottom       = core::recti(ix, iy, mx, my);

        w->m_skin_dest_area_top_left     = core::recti(ax, ay, ex, ey);
        w->m_skin_dest_area_top_right    = core::recti(cx, cy, gx, gy);
        w->m_skin_dest_area_bottom_left  = core::recti(hx, hy, lx, ly);
        w->m_skin_dest_area_bottom_right = core::recti(jx, jy, nx, ny);

        w->m_skin_dest_areas_inited = true;
    }

    if (vertical_flip)
    {
        if (!w->m_skin_dest_areas_yflip_inited)
        {
#define FLIP_Y( X ) {     const int y1 = X.UpperLeftCorner.Y \
                                       - w->m_skin_dest_y; \
const int y2 = X.LowerRightCorner.Y - w->m_skin_dest_y; \
X##_yflip = X; \
X##_yflip.UpperLeftCorner.Y = w->m_skin_dest_y + \
    (w->m_skin_dest_y2 - w->m_skin_dest_y) - y2;\
X##_yflip.LowerRightCorner.Y = w->m_skin_dest_y + \
    (w->m_skin_dest_y2 - w->m_skin_dest_y) - y1;}

        FLIP_Y(w->m_skin_dest_area_left)
        FLIP_Y(w->m_skin_dest_area_center)
        FLIP_Y(w->m_skin_dest_area_right)

        FLIP_Y(w->m_skin_dest_area_top)
        FLIP_Y(w->m_skin_dest_area_bottom)

        FLIP_Y(w->m_skin_dest_area_top_left)
        FLIP_Y(w->m_skin_dest_area_top_right)
        FLIP_Y(w->m_skin_dest_area_bottom_left)
        FLIP_Y(w->m_skin_dest_area_bottom_right)

#undef FLIP_Y
        }

        w->m_skin_dest_areas_yflip_inited = true;
        params.calculateYFlipIfNeeded();
    }

#define GET_AREA( X ) X = (vertical_flip ? params.X##_yflip : params.X)
    core::recti& GET_AREA(m_source_area_left);
    core::recti& GET_AREA(m_source_area_center);
    core::recti& GET_AREA(m_source_area_right);

    core::recti& GET_AREA(m_source_area_top);
    core::recti& GET_AREA(m_source_area_bottom);

    core::recti& GET_AREA(m_source_area_top_left);
    core::recti& GET_AREA(m_source_area_top_right);
    core::recti& GET_AREA(m_source_area_bottom_left);
    core::recti& GET_AREA(m_source_area_bottom_right);
#undef GET_AREA

#define GET_AREA( X ) X = (vertical_flip ? w->m_skin_##X##_yflip \
                                         : w->m_skin_##X)
    core::recti& GET_AREA(dest_area_left);
    core::recti& GET_AREA(dest_area_center);
    core::recti& GET_AREA(dest_area_right);

    core::recti& GET_AREA(dest_area_top);
    core::recti& GET_AREA(dest_area_bottom);

    core::recti& GET_AREA(dest_area_top_left);
    core::recti& GET_AREA(dest_area_top_right);
    core::recti& GET_AREA(dest_area_bottom_left);
    core::recti& GET_AREA(dest_area_bottom_right);
#undef GET_AREA

    SColor* colorptr = NULL;

    // create a color object
    if ( (w->m_skin_r != -1 && w->m_skin_g != -1 && w->m_skin_b != -1) ||
         ID_DEBUG || deactivated)
    {
        SColor thecolor(255, w->m_skin_r, w->m_skin_g, w->m_skin_b);
        colorptr = new SColor[4]();
        colorptr[0] = thecolor;
        colorptr[1] = thecolor;
        colorptr[2] = thecolor;
        colorptr[3] = thecolor;
    }

    // set it to transluscent
    if (ID_DEBUG || deactivated)
    {
        colorptr[0].setAlpha(100);
        colorptr[1].setAlpha(100);
        colorptr[2].setAlpha(100);
        colorptr[3].setAlpha(100);
    }

    if ((areas & BoxRenderParams::LEFT) != 0)
    {
        draw2DImage(source, dest_area_left,
                                            m_source_area_left, clipRect,
                                            colorptr, true /* alpha */);
    }

    if ((areas & BoxRenderParams::BODY) != 0)
    {
        draw2DImage(source, dest_area_center,
                                            m_source_area_center, clipRect,
                                            colorptr, true /* alpha */);
    }

    if ((areas & BoxRenderParams::RIGHT) != 0)
    {
        draw2DImage(source, dest_area_right,
                                            m_source_area_right, clipRect,
                                            colorptr, true /* alpha */);
    }

    if ((areas & BoxRenderParams::TOP) != 0)
    {
        draw2DImage(source, dest_area_top,
                                            m_source_area_top, clipRect,
                                            colorptr, true /* alpha */);
    }
    if ((areas & BoxRenderParams::BOTTOM) != 0)
    {
        draw2DImage(source, dest_area_bottom,
                                            m_source_area_bottom, clipRect,
                                            colorptr, true /* alpha */);
    }

    if ( ((areas & BoxRenderParams::LEFT) != 0) &&
         ((areas & BoxRenderParams::TOP ) != 0)     )
    {
        draw2DImage(source, dest_area_top_left,
                                            m_source_area_top_left, clipRect,
                                            colorptr, true /* alpha */);
    }
    if ( ((areas & BoxRenderParams::RIGHT) != 0) &&
         ((areas & BoxRenderParams::TOP  ) != 0)    )
    {
        draw2DImage(source, dest_area_top_right,
                                            m_source_area_top_right, clipRect,
                                            colorptr, true /* alpha */);
    }
    if ( ((areas & BoxRenderParams::LEFT  ) != 0) &&
         ((areas & BoxRenderParams::BOTTOM) != 0)    )
    {
        draw2DImage(source, dest_area_bottom_left,
                                            m_source_area_bottom_left,
                                            clipRect, colorptr,
                                            /*alpha*/true );
    }
    if ( ((areas & BoxRenderParams::RIGHT ) != 0) &&
         ((areas & BoxRenderParams::BOTTOM) != 0)    )
    {
        draw2DImage(source, dest_area_bottom_right,
                                            m_source_area_bottom_right,
                                            clipRect, colorptr,
                                            /*alpha*/true );
    }

    if (colorptr != NULL)
    {
        delete[] colorptr;
    }
#endif
}   // drawBoxFromStretchableTexture

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus
 *  for other players is not supported)
 */
void Skin::drawButton(Widget* w, const core::recti &rect,
                      const bool pressed, const bool focused)
{
    // if within an appearing dialog, grow
    if (m_dialog && m_dialog_size < 1.0f && w->m_parent != NULL &&
        w->m_parent->getType() == gui::EGUIET_WINDOW)
    {
        core::recti sized_rect = rect;
        core::position2d<u32> center =
            core::position2d<u32>(irr_driver->getFrameSize()/2);
        const float texture_size = sin(m_dialog_size*M_PI*0.5f);

        sized_rect.UpperLeftCorner.X  =
            center.X + (int)(((int)rect.UpperLeftCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.UpperLeftCorner.Y  =
            center.Y + (int)(((int)rect.UpperLeftCorner.Y
                            - (int)center.Y)*texture_size);
        sized_rect.LowerRightCorner.X =
            center.X + (int)(((int)rect.LowerRightCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.LowerRightCorner.Y =
            center.Y + (int)(((int)rect.LowerRightCorner.Y
                            - (int)center.Y)*texture_size);

        if (w->m_deactivated)
        {
            drawBoxFromStretchableTexture(w, sized_rect,
                                SkinConfig::m_render_params["button::deactivated"],
                                w->m_deactivated);
        }
        else if (focused)
        {
            drawBoxFromStretchableTexture(w, sized_rect,
                                SkinConfig::m_render_params["button::focused"],
                                w->m_deactivated);
        }
        else
        {
            drawBoxFromStretchableTexture(w, sized_rect,
                                SkinConfig::m_render_params["button::neutral"],
                                w->m_deactivated);
        }   // if not deactivated or focused
    }
    else   // not within an appearing dialog
    {
        if (w->m_deactivated)
        {
            drawBoxFromStretchableTexture(w, rect,
                                SkinConfig::m_render_params["button::deactivated"],
                                w->m_deactivated);
        }
        else if (focused)
        {
            drawBoxFromStretchableTexture(w, rect,
                                SkinConfig::m_render_params["button::focused"],
                                w->m_deactivated);
        }
        else
        {
            drawBoxFromStretchableTexture(w, rect,
                                SkinConfig::m_render_params["button::neutral"],
                                w->m_deactivated);
        }   // if not deactivated or focused
    }   // not within an appearing dialog
}   // drawButton

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus
 * for other players is not supported)
 */
void Skin::drawProgress(Widget* w, const core::recti &rect,
                        const bool pressed, const bool focused)
{
    core::recti sized_rect = rect;
    // if within an appearing dialog, grow
    if (m_dialog && m_dialog_size < 1.0f && w->m_parent != NULL &&
        w->m_parent->getType() == gui::EGUIET_WINDOW                 )
    {
        core::position2d<u32> center =
            core::position2d<u32>(irr_driver->getFrameSize()/2);
        const float texture_size = sin(m_dialog_size*M_PI*0.5f);

        sized_rect.UpperLeftCorner.X  =
            center.X + (int)(((int)rect.UpperLeftCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.UpperLeftCorner.Y  =
            center.Y + (int)(((int)rect.UpperLeftCorner.Y
                            - (int)center.Y)*texture_size);
        sized_rect.LowerRightCorner.X =
            center.X + (int)(((int)rect.LowerRightCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.LowerRightCorner.Y =
            center.Y + (int)(((int)rect.LowerRightCorner.Y
                            - (int)center.Y)*texture_size);

        drawBoxFromStretchableTexture(w, sized_rect,
                              SkinConfig::m_render_params["progress::neutral"],
                              w->m_deactivated);
    }
    else
    {
        ProgressBarWidget * progress = (ProgressBarWidget*)w;
        drawProgressBarInScreen(w, rect, progress->getValue(),
            w->m_deactivated);
    }
}   // drawProgress

// ----------------------------------------------------------------------------
void Skin::drawProgressBarInScreen(SkinWidgetContainer* swc,
                                   const core::rect< s32 > &rect, int progress,
                                   bool deactivated)
{
    drawBoxFromStretchableTexture(swc, rect,
        SkinConfig::m_render_params["progress::neutral"], deactivated);
    core::recti rect2 = rect;
    rect2.LowerRightCorner.X -= (rect.getWidth())
                              - progress * rect.getWidth() / 100;
    drawBoxFromStretchableTexture(swc, rect2,
        SkinConfig::m_render_params["progress::fill"], deactivated);
}   // drawProgress

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus
 * for other players is not supported)
 */
void Skin::drawRatingBar(Widget *w, const core::recti &rect,
                        const bool pressed, const bool focused)
{
#ifndef SERVER_ONLY
    RatingBarWidget *ratingBar = (RatingBarWidget*)w;

    const ITexture *texture = SkinConfig::m_render_params["rating::neutral"].getImage();
    const int texture_w = texture->getSize().Width / 4;
    const int texture_h = texture->getSize().Height;
    const float aspect_ratio = 1.0f;

    const int star_number = ratingBar->getStarNumber();

    int star_h = rect.getHeight();
    int star_w = (int)(aspect_ratio * star_h);

    if (rect.getWidth() < star_w * star_number)
    {
        const float scale_factor = rect.getWidth() / (float)(star_w * star_number);
        star_w = (int)(star_w * scale_factor);
        star_h = (int)(star_h * scale_factor);
    }

    // center horizontally and vertically
    const int x_from = rect.UpperLeftCorner.X;
    const int y_from = rect.UpperLeftCorner.Y;

    core::recti stars_rect(x_from, y_from, x_from + (star_number * star_w), y_from + star_h);

    if(!w->m_deactivated)
        ratingBar->setStepValuesByMouse(irr_driver->getDevice()->getCursorControl()->getPosition(), stars_rect);

    SColor colors[] =  { SColor(100,255,255,255),
                         SColor(100,255,255,255),
                         SColor(100,255,255,255),
                         SColor(100,255,255,255) };

    for (int i = 0; i < star_number; i++)
    {
        core::recti star_rect = rect;

        star_rect.UpperLeftCorner.X  = x_from + i * star_w;
        star_rect.UpperLeftCorner.Y  = y_from;
        star_rect.LowerRightCorner.X = x_from + (i + 1) * star_w;
        star_rect.LowerRightCorner.Y = y_from + star_h;

        int step = ratingBar->getStepsOfStar(i);

        const core::recti source_area(texture_w * step, 0,
                                      texture_w * (step + 1), texture_h);

        draw2DImage(texture,
                                            star_rect, source_area,
                                            0 /* no clipping */,
                                           (w->m_deactivated || ID_DEBUG) ? colors : 0,
                                            true /* alpha */);
    }
#endif
}   // drawRatingBar

// ----------------------------------------------------------------------------
SColor Skin::getColor(const std::string &name)
{
    return SkinConfig::m_colors[name];
}   // getColor

// ----------------------------------------------------------------------------
void Skin::drawRibbon(const core::recti &rect, Widget* widget,
                      const bool pressed, bool focused)
{
}   // drawRibbon

// ----------------------------------------------------------------------------
/**
  * @param focused whether this element is focus by the master player (whether
  * the widget is focused for other players is automatically determined)
  * FIXME: ugly to pass some focuses through parameter and others not xD
  */
void Skin::drawRibbonChild(const core::recti &rect, Widget* widget,
                           const bool pressed, bool focused)
{
#ifndef SERVER_ONLY
    // for now, when this kind of widget is disabled, just hide it. we can
    // change that behaviour if we ever need to...
    //if (widget->m_deactivated) return;

    bool mark_selected = widget->isSelected(PLAYER_ID_GAME_MASTER);

    IGUIElement* focusedElem = NULL;
    if (GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER) != NULL)
    {
        focusedElem = GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER)
                      ->getIrrlichtElement();
    }

    const bool parent_focused =
        (focusedElem == widget->m_event_handler->m_element);

    RibbonWidget* parentRibbon = (RibbonWidget*)widget->m_event_handler;
    RibbonType type = parentRibbon->getRibbonType();

    /* tab-bar ribbons */
    if (type == RIBBON_TABS)
    {
        video::SMaterial& material2D =
            irr_driver->getVideoDriver()->getMaterial2D();
        for (unsigned int n=0; n<MATERIAL_MAX_TEXTURES; n++)
        {
            material2D.UseMipMaps = false;
        }

        const bool mouseIn = rect.isPointInside(irr_driver->getDevice()
                                                ->getCursorControl()
                                                ->getPosition()        );

        BoxRenderParams* params;

        if (mark_selected && (focused || parent_focused))
            params = &SkinConfig::m_render_params["tab::focused"];
        else if (parentRibbon->m_mouse_focus == widget && mouseIn)
            params = &SkinConfig::m_render_params["tab::focused"];
        else if (mark_selected)
            params = &SkinConfig::m_render_params["tab::down"];
        else
            params = &SkinConfig::m_render_params["tab::neutral"];


        // automatically guess from position on-screen if tabs go up or down
        const bool vertical_flip =
            (unsigned int)rect.UpperLeftCorner.Y <
                irr_driver->getActualScreenSize().Height / 2;
        params->m_vertical_flip = vertical_flip;

        core::recti rect2 = rect;
        if (mark_selected)
        {
            // selected tab should be slighlty bigger than others
            if (vertical_flip) rect2.UpperLeftCorner.Y -= 10;
            else               rect2.LowerRightCorner.Y += 10;
        }

        drawBoxFromStretchableTexture(widget, rect2, *params,
                                      parentRibbon->m_deactivated ||
                                        widget->m_deactivated);

        for (unsigned int n=0; n<MATERIAL_MAX_TEXTURES; n++)
        {
            material2D.UseMipMaps = true;
        }
    }
    /* icon ribbons */
    else
    {
        bool use_glow = true;

        if (widget->m_event_handler != NULL &&
            widget->m_event_handler->m_properties[PROP_SQUARE] == "true")
            use_glow = false;
        if (widget->m_event_handler != NULL &&
            widget->m_event_handler->m_event_handler != NULL &&
            widget->m_event_handler->m_event_handler
                    ->m_properties[PROP_SQUARE] == "true")
            use_glow = false;

        /* in combo ribbons, always show selection */
        RibbonWidget* parentRibbonWidget = NULL;
        bool always_show_selection = false;

        if (widget->m_event_handler != NULL &&
            widget->m_event_handler->m_type == WTYPE_RIBBON)
        {
            parentRibbonWidget =
                dynamic_cast<RibbonWidget*>(widget->m_event_handler);
            if(parentRibbonWidget->getRibbonType() == RIBBON_COMBO)
                always_show_selection = true;
        }

        const bool mark_focused =
            focused || (parent_focused && parentRibbonWidget != NULL &&
                          parentRibbonWidget->m_mouse_focus == widget) ||
                       (mark_selected && !always_show_selection &&
                          parent_focused);

        /* draw "selection bubble" if relevant */
        if (always_show_selection && mark_selected)
        {
            ITexture* tex_bubble =
                SkinConfig::m_render_params["selectionHalo::neutral"]
                           .getImage();

            const int texture_w = tex_bubble->getSize().Width;
            const int texture_h = tex_bubble->getSize().Height;
            const float aspectRatio = (float)texture_w / (float)texture_h;

            core::recti source_area = core::recti(0, 0,
                                                          texture_w,texture_h);

            const float outgrow = 0.35f; // make slightly bigger than the
            // icon it's on
            const int rectHeight = int(rect.getHeight() * (1.0f + outgrow));
            const int rectWidth = int(rectHeight * aspectRatio);
            const int x_gap = (rect.getWidth() - rectWidth)/2;
            const int y_shift_up = int((rectHeight - rect.getHeight()) / 2.0f);

            core::position2di pos(rect.UpperLeftCorner.X + x_gap,
                                  rect.UpperLeftCorner.Y - y_shift_up);
            core::recti rect2(pos,
                              core::dimension2di(rectWidth, rectHeight) );

            if (widget->m_deactivated || ID_DEBUG)
            {
                SColor colors[] =  { SColor(100,255,255,255),
                                     SColor(100,255,255,255),
                                     SColor(100,255,255,255),
                                     SColor(100,255,255,255) };
                draw2DImage(tex_bubble, rect2,
                                                    source_area,
                                                    0 /* no clipping */,
                                                    colors, true /* alpha */);
            }
            else
            {
                draw2DImage(tex_bubble, rect2,
                                                    source_area,
                                                    0 /* no clipping */, 0,
                                                    true /* alpha */);
            }
        }

        // if multiple player selected the same ribbon item, we need to know
        // to make it visible
        int nPlayersOnThisItem = 0;

        if (mark_focused)
        {
            if (use_glow)
            {
                // don't mark filler items as focused
                if (widget->m_properties[PROP_ID] == RibbonWidget::NO_ITEM_ID)
                    return;

                int grow = 45;
                static float glow_effect = 0;


                const float dt = GUIEngine::getLatestDt();
                glow_effect += dt*3;
                if (glow_effect > 6.2832f /* 2*PI */) glow_effect -= 6.2832f;
                grow = (int)(45 + 10*sin(glow_effect));



                const int glow_center_x = rect.UpperLeftCorner.X
                                        + rect.getWidth()/2;
                const int glow_center_y = rect.UpperLeftCorner.Y
                                        + rect.getHeight() - 5;

                ITexture* tex_ficonhighlight =
                    SkinConfig::m_render_params["focusHalo::neutral"]
                               .getImage();
                const int texture_w = tex_ficonhighlight->getSize().Width;
                const int texture_h = tex_ficonhighlight->getSize().Height;

                core::recti source_area(0, 0, texture_w,texture_h);


                const core::recti rect2(glow_center_x - 45 - grow,
                                        glow_center_y - 25 - grow/2,
                                        glow_center_x + 45 + grow,
                                        glow_center_y + 25 + grow/2);

                draw2DImage(tex_ficonhighlight, rect2,
                                                    source_area,
                                                    /*clipping*/ 0,
                                                    /*color*/ 0,
                                                    /*alpha*/true );
            }
            // if we're not using glow, draw square focus instead
            else
            {
                const bool show_focus = (focused || parent_focused);

                if (!always_show_selection && !show_focus) return;

                // don't mark filler items as focused
                if (widget->m_properties[PROP_ID] == RibbonWidget::NO_ITEM_ID)
                    return;

                drawBoxFromStretchableTexture(parentRibbonWidget, rect,
                      SkinConfig::m_render_params["squareFocusHalo::neutral"]);
                nPlayersOnThisItem++;
            }
        } // end if mark_focused

        // ---- Draw selection for other players than player 1
        if (parentRibbon->isFocusedForPlayer(1) &&
            parentRibbon->getSelectionIDString(1) ==
                                            widget->m_properties[PROP_ID])
        {
            if (nPlayersOnThisItem > 0)
            {
                core::recti rect2 =  rect;
                const int enlarge = nPlayersOnThisItem*6;
                rect2.UpperLeftCorner.X -= enlarge;
                rect2.UpperLeftCorner.Y -= enlarge;
                rect2.LowerRightCorner.X += enlarge;
                rect2.LowerRightCorner.Y += enlarge;
                drawBoxFromStretchableTexture(parentRibbonWidget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo2::neutral"]);
            }
            else
            {
                drawBoxFromStretchableTexture(parentRibbonWidget, rect,
                     SkinConfig::m_render_params["squareFocusHalo2::neutral"]);
            }

            nPlayersOnThisItem++;
        }

        if (parentRibbon->isFocusedForPlayer(2) &&
            parentRibbon->getSelectionIDString(2) ==
                                                 widget->m_properties[PROP_ID])
        {
            if (nPlayersOnThisItem > 0)
            {
                core::recti rect2 =  rect;
                const int enlarge = nPlayersOnThisItem*6;
                rect2.UpperLeftCorner.X -= enlarge;
                rect2.UpperLeftCorner.Y -= enlarge;
                rect2.LowerRightCorner.X += enlarge;
                rect2.LowerRightCorner.Y += enlarge;
                drawBoxFromStretchableTexture(parentRibbonWidget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo3::neutral"]);
            }
            else
            {
                drawBoxFromStretchableTexture(parentRibbonWidget, rect,
                     SkinConfig::m_render_params["squareFocusHalo3::neutral"]);
            }
            nPlayersOnThisItem++;
        }

        if (parentRibbon->isFocusedForPlayer(3) &&
            parentRibbon->getSelectionIDString(3) ==
                                                 widget->m_properties[PROP_ID])
        {
            if (nPlayersOnThisItem > 0)
            {
                core::recti rect2 =  rect;
                const int enlarge = nPlayersOnThisItem*6;
                rect2.UpperLeftCorner.X -= enlarge;
                rect2.UpperLeftCorner.Y -= enlarge;
                rect2.LowerRightCorner.X += enlarge;
                rect2.LowerRightCorner.Y += enlarge;
                drawBoxFromStretchableTexture(parentRibbonWidget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo4::neutral"]);
            }
            else
            {
                drawBoxFromStretchableTexture(parentRibbonWidget, rect,
                     SkinConfig::m_render_params["squareFocusHalo4::neutral"]);
            }
            nPlayersOnThisItem++;
        }

        drawIconButton(rect, widget, pressed, focused);

    } // end if icon ribbons


    if (/*mark_selected && widget->hasTooltip() &&
        (focused || parent_focused) &&*/
        parentRibbon->m_mouse_focus == widget)
    {
        if (rect.isPointInside(irr_driver->getDevice()->getCursorControl()
                                                      ->getPosition()))
        {
            m_tooltip_at_mouse.push_back(true);
            m_tooltips.push_back(widget);
        }
    }
#endif
}   // drawRibbonChild

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (whether
 *  the widget is focused for other players is automatically determined)
 * FIXME: ugly to pass some focuses through parameter and others not xD
 */
void Skin::drawSpinnerBody(const core::recti &rect, Widget* widget,
                           const bool pressed, bool focused)
{
    if (!widget->isVisible()) return;

    if (!focused)
    {
        IGUIElement* focused_widget = NULL;

        const int playerID = 0;
        if (GUIEngine::getFocusForPlayer(playerID) != NULL)
        {
            focused_widget =
                GUIEngine::getFocusForPlayer(playerID)->getIrrlichtElement();
        }
        if (focused_widget != NULL && widget->m_children.size()>2)
        {
            if (widget->m_children[0].getID() == focused_widget->getID() ||
                widget->m_children[1].getID() == focused_widget->getID() ||
                widget->m_children[2].getID() == focused_widget->getID())
            {
                focused = true;
            }
        }
    }

    BoxRenderParams* params;
    SpinnerWidget* q = dynamic_cast<SpinnerWidget*>(widget);
    if(q->getUseBackgroundColor())
    {
        int player_id=q->getSpinnerWidgetPlayerID();
        if(player_id==0)
            params=&SkinConfig::m_render_params["spinner1::neutral"];
        else if(player_id==1)
            params=&SkinConfig::m_render_params["spinner2::neutral"];
        else if(player_id==2)
            params=&SkinConfig::m_render_params["spinner3::neutral"];
        else if(player_id==3)
            params=&SkinConfig::m_render_params["spinner4::neutral"];
        else
        {
            Log::fatal("Skin::drawSpinnerBody", "Unknown playerID (more than 4 players?)");
            // Silence compiler warning
            params = NULL;
        }
    }
    else if (widget->m_deactivated)
    {
        params=&SkinConfig::m_render_params["spinner::deactivated"];
    }
    else if (focused || pressed)
    {
        params=&SkinConfig::m_render_params["spinner::focused"];
    }
    else
    {
        params=&SkinConfig::m_render_params["spinner::neutral"];
    }
    if (widget->isFocusedForPlayer(0))
    {
        core::recti rect2 = rect;
        rect2.UpperLeftCorner.X += 2;
        rect2.UpperLeftCorner.Y -= 3;
        rect2.LowerRightCorner.X -= 2;
        rect2.LowerRightCorner.Y += 5;
        drawBoxFromStretchableTexture(widget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo::neutral"]);


    }
    else if (widget->isFocusedForPlayer(1))
    {
        core::recti rect2 = rect;
        rect2.UpperLeftCorner.X += 2;
        rect2.UpperLeftCorner.Y -= 3;
        rect2.LowerRightCorner.X -= 2;
        rect2.LowerRightCorner.Y += 5;
        drawBoxFromStretchableTexture(widget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo2::neutral"]);
    }
    else if (widget->isFocusedForPlayer(2))
    {
        core::recti rect2 = rect;
        rect2.UpperLeftCorner.X += 2;
        rect2.UpperLeftCorner.Y -= 3;
        rect2.LowerRightCorner.X -= 2;
        rect2.LowerRightCorner.Y += 5;
        drawBoxFromStretchableTexture(widget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo3::neutral"]);
    }
    else if (widget->isFocusedForPlayer(3))
    {
        core::recti rect2 = rect;
        rect2.UpperLeftCorner.X += 2;
        rect2.UpperLeftCorner.Y -= 3;
        rect2.LowerRightCorner.X -= 2;
        rect2.LowerRightCorner.Y += 5;
        drawBoxFromStretchableTexture(widget, rect2,
                     SkinConfig::m_render_params["squareFocusHalo4::neutral"]);
    }

    core::recti sized_rect = rect;
    if (m_dialog && m_dialog_size < 1.0f && widget->m_parent != NULL &&
        widget->m_parent->getType() == gui::EGUIET_WINDOW)
    {
        core::position2d<u32> center(irr_driver->getFrameSize()/2);
        const float texture_size = sin(m_dialog_size*M_PI*0.5f);
        sized_rect.UpperLeftCorner.X  =
            center.X + (int)(((int)rect.UpperLeftCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.UpperLeftCorner.Y  =
            center.Y + (int)(((int)rect.UpperLeftCorner.Y
                            - (int)center.Y)*texture_size);

        sized_rect.LowerRightCorner.X =
            center.X + (int)(((int)rect.LowerRightCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.LowerRightCorner.Y =
            center.Y + (int)(((int)rect.LowerRightCorner.Y
                            - (int)center.Y)*texture_size);
    }

    drawBoxFromStretchableTexture(widget, sized_rect, *params,
                                  widget->m_deactivated);


    // ---- If this spinner is of "gauge" type, draw filling
    const SpinnerWidget* w = dynamic_cast<const SpinnerWidget*>(widget);

    if (w->isGauge() && !w->m_deactivated)
    {
        const int handle_size = (int)( widget->m_h*params->m_left_border
                                 /(float)params->getImage()->getSize().Height );
        const float value = (float)(w->getValue() - w->getMin())
                          / (w->getMax() - w->getMin());


        const core::recti dest_area(rect.UpperLeftCorner.X + handle_size,
                                    rect.UpperLeftCorner.Y,
                                    rect.UpperLeftCorner.X + handle_size +
                                           (int)((widget->m_w
                                                  - 2*handle_size)*value),
                                    rect.UpperLeftCorner.Y + widget->m_h);

        const ITexture* texture =
            SkinConfig::m_render_params["gaugefill::neutral"].getImage();
        const int texture_w = texture->getSize().Width;
        const int texture_h = texture->getSize().Height;

        const core::recti source_area(0, 0, texture_w, texture_h);

        draw2DImage(texture,
                    dest_area, source_area,
                    0 /* no clipping */, 0,
                    true /* alpha */);

    }

    if (focused && widget->hasTooltip())
    {
        m_tooltip_at_mouse.push_back(false);
        m_tooltips.push_back(widget);
    }
}

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus for
 *   other players is not supported)
 */
void Skin::drawSpinnerChild(const core::recti &rect, Widget* widget,
                            const bool pressed, bool focused)
{
    if (!widget->isVisible()) return;

    if (!widget->m_deactivated && pressed)
    {
        Widget* spinner = widget->m_event_handler;
        int areas = 0;

        if (widget->m_properties[PROP_ID] == "left")
            areas = BoxRenderParams::LEFT;
        else if (widget->m_properties[PROP_ID] == "right")
            areas = BoxRenderParams::RIGHT;
        else
            return;

        core::recti rect2(spinner->m_x, spinner->m_y,
                          spinner->m_x + spinner->m_w,
                          spinner->m_y + spinner->m_h  );

        BoxRenderParams& params = SkinConfig::m_render_params["spinner::down"];
        params.areas = areas;
        drawBoxFromStretchableTexture(widget, rect, params,
                                      widget->m_deactivated);
    }

}

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus for
*   other players is not supported)
 */
void Skin::drawIconButton(const core::recti &rect, Widget* widget,
                          const bool pressed, bool focused)
{
#ifndef SERVER_ONLY
    RibbonWidget* parentRibbon = dynamic_cast<RibbonWidget*>(widget->m_event_handler);
    IGUIElement* focusedElem = NULL;
    if (GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER) != NULL)
    {
        focusedElem = GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER)
                                    ->getIrrlichtElement();
    }
    const bool parent_focused = (widget->m_event_handler == NULL ? false :
        (focusedElem == widget->m_event_handler->m_element));
    const bool mark_focused =
        focused || (parent_focused && parentRibbon != NULL &&
                    parentRibbon->getSelectionIDString(PLAYER_ID_GAME_MASTER) == widget->m_properties[PROP_ID]);

    if (focused)
    {
        int grow = 45;
        static float glow_effect = 0;

        const float dt = GUIEngine::getLatestDt();
        glow_effect += dt*3;
        if (glow_effect > 6.2832f /* 2*PI */) glow_effect -= 6.2832f;
        grow = (int)(45 + 10*sin(glow_effect));

        const int glow_center_x = rect.UpperLeftCorner.X+rect.getWidth()/2;
        const int glow_center_y = rect.LowerRightCorner.Y;

        ITexture* tex_ficonhighlight =
            SkinConfig::m_render_params["focusHalo::neutral"].getImage();
        const int texture_w = tex_ficonhighlight->getSize().Width;
        const int texture_h = tex_ficonhighlight->getSize().Height;

        core::recti source_area = core::recti(0, 0, texture_w, texture_h);


        const core::recti rect2(glow_center_x - 45 - grow,
                                glow_center_y - 25 - grow/2,
                                glow_center_x + 45 + grow,
                                glow_center_y + 25 + grow/2);

        draw2DImage(tex_ficonhighlight, rect2,
                                            source_area,
                                            0 /* no clipping */, 0,
                                            true /* alpha */);
    }

    core::recti sized_rect = rect;
    if (m_dialog && m_dialog_size < 1.0f && widget->m_parent != NULL &&
        widget->m_parent->getType() == gui::EGUIET_WINDOW)
    {
        core::position2d<u32> center(irr_driver->getFrameSize()/2);
        const float texture_size = sin(m_dialog_size*M_PI*0.5f);
        sized_rect.UpperLeftCorner.X  =
            center.X + (int)(((int)rect.UpperLeftCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.UpperLeftCorner.Y  =
            center.Y + (int)(((int)rect.UpperLeftCorner.Y
                            - (int)center.Y)*texture_size);

        sized_rect.LowerRightCorner.X =
            center.X + (int)(((int)rect.LowerRightCorner.X
                            - (int)center.X)*texture_size);
        sized_rect.LowerRightCorner.Y =
            center.Y + (int)(((int)rect.LowerRightCorner.Y
                            - (int)center.Y)*texture_size);
    }

    IconButtonWidget* icon_widget = (IconButtonWidget*) widget;

    if (widget->m_type == WTYPE_MODEL_VIEW)
    {
        // Model view widgets don't generate mipmaps so disable material 2D
        irr_driver->getVideoDriver()->enableMaterial2D(false);
    }


    if (widget->m_deactivated || ID_DEBUG)
    {
        SColor colors[] =  { SColor(100,255,255,255),
                             SColor(100,255,255,255),
                             SColor(100,255,255,255),
                             SColor(100,255,255,255) };
        core::recti r(0,0,icon_widget->m_texture_w, icon_widget->m_texture_h);
        draw2DImage(icon_widget->getTexture(), sized_rect,
                                            r, 0 /* no clipping */, colors,
                                            true /* alpha */);
    }
    else
    {
        const video::ITexture* t = icon_widget->getTexture();

        const bool mouseInside =
            rect.isPointInside(irr_driver->getDevice()->getCursorControl()
                                                      ->getPosition());
        if (icon_widget->m_highlight_texture != NULL &&
            (mark_focused || mouseInside)                   )
        {
            t = icon_widget->m_highlight_texture;
        }
        core::recti r(0,0,icon_widget->m_texture_w, icon_widget->m_texture_h);
        draw2DImage(t, sized_rect, r,0
                                            /* no clipping */, 0,
                                            true /* alpha */);
    }

    if (widget->m_type == WTYPE_MODEL_VIEW)
    {
        irr_driver->getVideoDriver()->enableMaterial2D();
    }
#endif
}   // drawIconButton

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus
*   for other players is not supported)
 */
void Skin::drawCheckBox(const core::recti &rect, Widget* widget, bool focused)
{
    CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

    ITexture* texture;

    if (w->getState() == true)
    {
        if (w->m_deactivated)
        {
            texture = SkinConfig::m_render_params["checkbox::deactivated+checked"]
                .getImage();
        }
        else if(focused)
        {
            texture = SkinConfig::m_render_params["checkbox::focused+checked"]
                .getImage();
        }
        else
        {
            texture = SkinConfig::m_render_params["checkbox::neutral+checked"]
                .getImage();
        }
    }
    else
    {
        if (w->m_deactivated)
        {
            texture = SkinConfig::m_render_params["checkbox::deactivated+unchecked"]
                .getImage();
        }
        else if(focused)
        {
            texture = SkinConfig::m_render_params["checkbox::focused+unchecked"]
                .getImage();
        }
        else
        {
            texture = SkinConfig::m_render_params["checkbox::neutral+unchecked"]
                .getImage();
        }
    }

    const int texture_w = texture->getSize().Width;
    const int texture_h = texture->getSize().Height;

    const core::recti source_area = core::recti(0, 0, texture_w, texture_h);

    if (widget->m_deactivated)
    {
        SColor colors[] =  { SColor(100,255,255,255),
                             SColor(100,255,255,255),
                             SColor(100,255,255,255),
                             SColor(100,255,255,255) };
        draw2DImage(texture, rect, source_area,
                    0 /* no clipping */, colors,
                    true /* alpha */);
    }
    else
    {
        draw2DImage(texture, rect, source_area,
                    0 /* no clipping */, 0,
                    true /* alpha */);
    }


    if (focused && widget->hasTooltip())
    {
        const core::position2di mouse_position =
            irr_driver->getDevice()->getCursorControl()->getPosition();

        if (rect.isPointInside(mouse_position))
        {
            m_tooltip_at_mouse.push_back(true);
            m_tooltips.push_back(widget);
        }
    }
}   // drawCheckBox

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus
 *  for other players is not supported)
 */
void Skin::drawList(const core::recti &rect, Widget* widget, bool focused)
{
    //drawBoxFromStretchableTexture(widget, rect,
    //                              SkinConfig::m_render_params["list::neutral"],
    //                              widget->m_deactivated, NULL);

}   // drawList

// ----------------------------------------------------------------------------
/**
 * @param focused whether this element is focus by the master player (focus for
*   other players is not supported)
 */
void Skin::drawListSelection(const core::recti &rect, Widget* widget,
                             bool focused, const core::recti *clip)
{
    ListWidget* list = dynamic_cast<ListWidget*>(widget);
    assert(list != NULL);

    drawBoxFromStretchableTexture(&list->m_selection_skin_info, rect,
                                  SkinConfig::m_render_params["listitem::focused"],
                                  list->m_deactivated, clip);
}   // drawListSelection

// ----------------------------------------------------------------------------
void Skin::drawListHeader(const irr::core::rect< irr::s32 > &rect,
                          Widget* widget)
{
#ifndef SERVER_ONLY
    bool isSelected =
         (((ListWidget*)widget->m_event_handler)->m_selected_column == widget &&
         ((ListWidget*)widget->m_event_handler)->m_sort_default == false);

    drawBoxFromStretchableTexture(widget, rect,
            (isSelected ? SkinConfig::m_render_params["list_header::down"]
                        : SkinConfig::m_render_params["list_header::neutral"]),
            false, NULL /* clip */);

    if (isSelected)
    {
        /** \brief img sets the icon for the column according to sort order **/
        ITexture* img;
        if (((ListWidget*)widget->m_event_handler)->m_sort_desc)
            img =
                SkinConfig::m_render_params["list_sort_up::neutral"].getImage();
        else
            img =
                SkinConfig::m_render_params["list_sort_down::neutral"].getImage();

        core::recti destRect(rect.UpperLeftCorner,
                             core::dimension2di(rect.getHeight(),
                                                rect.getHeight()));
        core::recti srcRect(core::position2d<s32>(0,0), img->getSize());
        draw2DImage(img, destRect, srcRect,
                                                  NULL, NULL, /* alpha */true);
    }
#endif
}   // drawListHeader

// ----------------------------------------------------------------------------
/** recursive function to render all sections (recursion allows to easily
 *  traverse the tree of children and sub-children)
 */
void Skin::renderSections(PtrVector<Widget>* within_vector)
{
#ifndef SERVER_ONLY
    if (within_vector == NULL) within_vector = &getCurrentScreen()->m_widgets;

    const unsigned short widgets_amount = within_vector->size();

    for(int n=0; n<widgets_amount; n++)
    {
        Widget& widget = (*within_vector)[n];

        if (widget.m_type == WTYPE_DIV)
        {
            if (widget.m_show_bounding_box)
            {
                if (widget.m_is_bounding_box_round)
                {
                    core::recti rect(widget.m_x, widget.m_y,
                                     widget.m_x + widget.m_w,
                                     widget.m_y + widget.m_h );
                    drawBoxFromStretchableTexture(&widget, rect,
                      SkinConfig::m_render_params["rounded_section::neutral"]);
                }
                else
                {
                    core::recti rect(widget.m_x, widget.m_y,
                                     widget.m_x + widget.m_w,
                                     widget.m_y + widget.m_h );
                    drawBoxFromStretchableTexture(&widget, rect,
                              SkinConfig::m_render_params["section::neutral"]);
                }

                renderSections( &widget.m_children );
            }
            else if (widget.isBottomBar())
            {
                const core::dimension2du &framesize =
                                                   irr_driver->getFrameSize();

                // bar.png is 128 pixels high
                const float y_size = (framesize.Height - widget.m_y) / 128.0f;

                // there's about 40 empty pixels at the top of bar.png
                ITexture* tex =
                    irr_driver->getTexture(FileManager::GUI,"bar.png");
                if(!tex)
                {
                    tex = irr_driver->getTexture(FileManager::GUI, "main_help.png");
                    if(!tex)
                        Log::fatal("Skin",
                        "Can't find fallback texture 'main_help.png, aborting.");
                }
                core::recti r1(0, (int)(widget.m_y - 40*y_size),
                               framesize.Width, framesize.Height);
                core::recti r2(core::dimension2di(0,0), tex->getSize());
                draw2DImage(tex, r1, r2,
                                                          0, 0, /*alpha*/true);
            }
            else if (widget.isTopBar())
            {
                ITexture* tex =
                    irr_driver->getTexture(FileManager::GUI, "top_bar.png");

                core::recti r1(0,               0,
                               (int)widget.m_w, (int)widget.m_h);
                core::recti r2(core::dimension2di(0,0), tex->getSize());
                draw2DImage(tex, r1, r2,
                                                          0, 0, /*alpha*/false);
            }
            else
            {
                renderSections( &widget.m_children );
            }
        }
    } // next
#endif   // !SERVER_ONLY
}   // renderSections

// ----------------------------------------------------------------------------
void Skin::drawScrollbarBackground(const irr::core::rect< irr::s32 > &rect)
{
#ifndef SERVER_ONLY
    // leave square space at both ends for up/down buttons (yeah, irrlicht
    // doesn't handle that)
    core::recti rect2 = rect;
    rect2.UpperLeftCorner.Y  += rect.getWidth();
    rect2.LowerRightCorner.Y -= rect.getWidth();

    BoxRenderParams& p =
        SkinConfig::m_render_params["scrollbar_background::neutral"];

    draw2DImage(p.getImage(), rect2,
                                        p.m_source_area_center,
                                        0 /* no clipping */, 0,
                                        true /* alpha */);
#endif
}   // drawScrollbarBackground

// ----------------------------------------------------------------------------
void Skin::drawScrollbarThumb(const irr::core::rect< irr::s32 > &rect)
{
#ifndef SERVER_ONLY
    BoxRenderParams& p =
        SkinConfig::m_render_params["scrollbar_thumb::neutral"];

    draw2DImage(p.getImage(), rect,
                                        p.m_source_area_center,
                                        0 /* no clipping */, 0,
                                        true /* alpha */);
#endif
}   // drawScrollbarThumb

// ----------------------------------------------------------------------------
void Skin::drawScrollbarButton(const irr::core::rect< irr::s32 > &rect,
                               const bool pressed, const bool bottomArrow)
{
    BoxRenderParams& p = (pressed)
                    ? SkinConfig::m_render_params["scrollbar_button::down"]
                    : SkinConfig::m_render_params["scrollbar_button::neutral"];

    if (!bottomArrow)
    {
        draw2DImage(p.getImage(), rect,
                                            p.m_source_area_center,
                                            0 /* no clipping */, 0,
                                            true /* alpha */);
    }
    else
    {
        // flip image
        const irr::core::rect<irr::s32>& source_area = p.m_source_area_center;
        const int x0 = source_area.UpperLeftCorner.X;
        const int x1 = source_area.LowerRightCorner.X;
        const int y0 = source_area.UpperLeftCorner.Y;
        const int y1 = source_area.LowerRightCorner.Y;

        draw2DImage(p.getImage(), rect,
                                            core::recti(x0, y1, x1, y0),
                                            0 /* no clipping */, 0,
                                            true /* alpha */);
    }

}   // drawScrollbarButton

// ----------------------------------------------------------------------------
void Skin::drawTooltips()
{
    for (unsigned int n=0; n<m_tooltips.size(); n++)
    {
        drawTooltip(m_tooltips[n], m_tooltip_at_mouse[n]);
    }
    m_tooltips.clear();
    m_tooltip_at_mouse.clear();
}   // drawTooltips

// ----------------------------------------------------------------------------
void Skin::drawTooltip(Widget* widget, bool atMouse)
{
    if (widget->getTooltipText().size() == 0) return;

    irr::gui::ScalableFont* font = GUIEngine::getSmallFont();
    core::dimension2d<u32> size =
        font->getDimension(widget->getTooltipText().c_str());
    core::position2di pos(widget->m_x + 15, widget->m_y + widget->m_h);

    if (atMouse)
    {
        pos = irr_driver->getDevice()->getCursorControl()->getPosition()
            + core::position2di(10 - size.Width / 2, 20);
    }

    core::recti r(pos, size);
    drawBoxFromStretchableTexture(widget, r,
                              SkinConfig::m_render_params["tooltip::neutral"]);
    font->draw(widget->getTooltipText(), r, video::SColor(255, 0, 0, 0),
               false, false);
}   // drawTooltip

#if 0
#pragma mark -
#pragma mark irrlicht skin functions
#endif

// ----------------------------------------------------------------------------
void Skin::draw2DRectangle (IGUIElement *element, const video::SColor &color,
                            const core::recti &rect, const core::recti *clip)
{
    if (GUIEngine::getStateManager()->getGameState() == GUIEngine::GAME)
        return; // ignore in game mode

    if (element->getType() == gui::EGUIET_SCROLL_BAR)
    {
        drawScrollbarBackground(rect);
        return;
    }

    const int id = element->getID();

    Widget* widget = GUIEngine::getWidget(id);
    if (widget == NULL) return;

    const WidgetType type = widget->m_type;
    if (type == WTYPE_LIST)
    {
        // lists not supported in multiplayer screens
        const bool focused =
            GUIEngine::isFocusedForPlayer(widget, PLAYER_ID_GAME_MASTER);

        drawListSelection(rect, widget, focused, clip);
    }
}   // draw2DRectangle

// -----------------------------------------------------------------------------
void Skin::process3DPane(IGUIElement *element, const core::recti &rect,
                         const bool pressed)
{
    const int id = element->getID();

    Widget* widget = NULL;
    if (id != -1) widget = GUIEngine::getWidget(id);

    if (widget == NULL)
    {
        if (element->getType() == gui::EGUIET_BUTTON &&
            element->getParent() != NULL &&
            element->getParent()->getType() == EGUIET_SCROLL_BAR)
        {
            const int parentHeight =
                element->getParent()->getRelativePosition().getHeight();
            const int y = element->getRelativePosition().UpperLeftCorner.Y;

            const bool bottomButton = (y > parentHeight/2);

            drawScrollbarButton(rect, pressed, bottomButton);
        }

        return;
    }

    const bool focused =
        GUIEngine::isFocusedForPlayer(widget, PLAYER_ID_GAME_MASTER);

    if (widget == NULL) return;

    const WidgetType type = widget->m_type;

    // buttons are used for other uses than plain clickable buttons because
    // irrLicht does not have widgets for everything we need. so at render
    // time, we just check which type this button represents and render
    // accordingly

    if (widget->m_event_handler != NULL &&
        widget->m_event_handler->m_type == WTYPE_RIBBON)
    {
        drawRibbonChild(rect, widget, pressed, focused);
    }
    else if (widget->m_event_handler != NULL &&
        widget->m_event_handler->m_type == WTYPE_SPINNER)
    {
        if (!widget->m_event_handler->m_deactivated)
            drawSpinnerChild(rect, widget, pressed, focused);
    }
    else if (type == WTYPE_MODEL_VIEW)
    {
        ModelViewWidget* mvw = dynamic_cast<ModelViewWidget*>(widget);
#ifndef SERVER_ONLY
        mvw->drawRTTScene(rect);
#endif
    }
    else if (type == WTYPE_ICON_BUTTON)
    {
        drawIconButton(rect, widget, pressed, focused);
    }
    else if (type == WTYPE_BUTTON)
    {
        if (widget->m_event_handler != NULL &&
            widget->m_event_handler->getType() == WTYPE_LIST)
        {
            drawListHeader(rect, widget);
        }
        else
        {
            drawButton(widget, rect, pressed, focused);
        }
    }
    else if(type == WTYPE_PROGRESS)
    {
        drawProgress(widget, rect, pressed, focused);
    }
    else if(type == WTYPE_RIBBON)
    {
        drawRibbon(rect, widget, pressed, focused);
    }
    else if(type == WTYPE_SPINNER)
    {
        drawSpinnerBody(rect, widget, pressed, focused);
    }
    else if(type == WTYPE_CHECKBOX)
    {
        drawCheckBox(rect, widget, focused);
    }
    else if(type == WTYPE_RATINGBAR)
    {
        drawRatingBar(widget, rect, pressed, focused);
    }


    if (ID_DEBUG && id != -1 && Widget::isFocusableId(id))
    {
        irr::core::stringw idstring;
        idstring += id;
        SColor color(255, 255, 0, 0);
        GUIEngine::getFont()->draw(idstring.c_str(), rect, color, true, true);
    }

    if (widget->m_badges != 0)
    {
        drawBadgeOn(widget, rect+position2d<s32>(widget->m_badge_x_shift, 0));
    }
}   // process3DPane

// -----------------------------------------------------------------------------
void doDrawBadge(ITexture* texture, const core::recti& rect,
                 float max_icon_size, bool badge_at_left)
{
#ifndef SERVER_ONLY
    // In case of a problem
    if(!texture) return;

    const core::dimension2d<u32>& texture_size = texture->getSize();
    const float aspectRatio = (float)texture_size.Width
                            / (float)texture_size.Height;
    const int h = rect.getHeight() <= 50
                ? rect.getHeight()
                : std::min( (int)(rect.getHeight()*max_icon_size),
                            (int)(texture_size.Height)             );
    int w = (int)(aspectRatio*h);

    const core::recti source_area(0, 0, texture_size.Width,
                                  texture_size.Height);

    const core::recti rect2(badge_at_left ? rect.UpperLeftCorner.X
                                          : rect.LowerRightCorner.X - w,
                            rect.LowerRightCorner.Y - h,
                            badge_at_left ? rect.UpperLeftCorner.X + w
                                          : rect.LowerRightCorner.X,
                            rect.LowerRightCorner.Y                     );

    draw2DImage(texture, rect2, source_area,
                                        0 /* no clipping */, 0,
                                        true /* alpha */);
#endif
}  // doDrawBadge

// ----------------------------------------------------------------------------
void Skin::drawBadgeOn(const Widget* widget, const core::recti& rect)
{
    if (widget->m_badges & LOCKED_BADGE)
    {
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "gui_lock.png");
        float max_icon_size = 0.5f; // Lock badge can be quite big
        doDrawBadge(texture, rect, max_icon_size, true);
    }
    if (widget->m_badges & OK_BADGE)
    {
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "green_check.png");
        float max_icon_size = 0.35f;
        doDrawBadge(texture, rect, max_icon_size, true);
    }
    if (widget->m_badges & BAD_BADGE)
    {
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "red_mark.png");
        float max_icon_size = 0.35f;
        doDrawBadge(texture, rect, max_icon_size, false);
    }
    if (widget->m_badges & TROPHY_BADGE)
    {
        float max_icon_size = 0.43f;
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "cup_bronze.png");
        doDrawBadge(texture, rect, max_icon_size, false);
    }
    if (widget->m_badges & KEYBOARD_BADGE)
    {
        float max_icon_size = 0.43f;
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "keyboard.png");
        doDrawBadge(texture, rect, max_icon_size, true);
    }
    if (widget->m_badges & GAMEPAD_BADGE)
    {
        float max_icon_size = 0.43f;
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "gamepad.png");
        doDrawBadge(texture, rect, max_icon_size, true);
    }
    if (widget->m_badges & LOADING_BADGE)
    {
        float max_icon_size = 0.43f;
        video::ITexture* texture = irr_driver->getTexture(FileManager::GUI,
                                                          "hourglass.png");
        doDrawBadge(texture, rect, max_icon_size, true);
    }
    if (widget->m_badges & ZIPPER_BADGE)
    {
        float max_icon_size = 0.43f;
        video::ITexture* texture = irr_driver->getTexture(FileManager::MODEL,
                                                          "zipper_collect.png");
        doDrawBadge(texture, rect, max_icon_size, false);
    }
    if (widget->m_badges & ANCHOR_BADGE)
    {
        float max_icon_size = 0.43f;
        video::ITexture* texture = irr_driver->getTexture(FileManager::MODEL,
                                                          "anchor-icon.png");
        doDrawBadge(texture, rect, max_icon_size, false);
    }
}   // drawBadgeOn

// -----------------------------------------------------------------------------
void Skin::draw3DButtonPanePressed (IGUIElement *element,
                                    const core::recti &rect,
                                    const core::recti *clip)
{
    process3DPane(element, rect, true /* pressed */ );
}   // draw3DButtonPanePressed

// -----------------------------------------------------------------------------
void Skin::draw3DButtonPaneStandard (IGUIElement *element,
                                     const core::recti &rect,
                                     const core::recti *clip)
{
    if (element->getType()==gui::EGUIET_SCROLL_BAR)
    {
        drawScrollbarThumb(rect);
    }
    else
    {
        process3DPane(element, rect, false /* pressed */ );
    }
}   // draw3DButtonPaneStandard

// -----------------------------------------------------------------------------
void Skin::draw3DSunkenPane (IGUIElement *element, video::SColor bgcolor,
                             bool flat, bool fillBackGround,
                             const core::recti &rect, const core::recti *clip)
{
#ifndef SERVER_ONLY
    const int id = element->getID();
    Widget* widget = GUIEngine::getWidget(id);

    if (widget == NULL) return;

    const WidgetType type = widget->m_type;

    IGUIElement* focusedElem = NULL;
    if (GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER) != NULL)
    {
        focusedElem = GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER)
                                 ->getIrrlichtElement();
    }

    const bool focused = (focusedElem == element);

    if (element->getType()==gui::EGUIET_EDIT_BOX)
    {
        SColor& bg_color = SkinConfig::m_colors["text_field::background"];
        SColor& bg_color_focused = SkinConfig::m_colors["text_field::background_focused"];
        SColor& bg_color_deactivated = SkinConfig::m_colors["text_field::background_deactivated"];
        SColor& border_color = SkinConfig::m_colors["text_field::neutral"];
        SColor& border_color_focus = SkinConfig::m_colors["text_field::focused"];
        SColor& border_color_deactivated = SkinConfig::m_colors["text_field::deactivated"];

        core::recti borderArea = rect;
        //borderArea.UpperLeftCorner -= position2d< s32 >( 2, 2 );
        //borderArea.LowerRightCorner += position2d< s32 >( 2, 2 );

        // if within an appearing dialog, grow
        if (m_dialog && m_dialog_size < 1.0f && widget->m_parent != NULL &&
            widget->m_parent->getType() == gui::EGUIET_WINDOW)
        {
            core::position2d<u32> center(irr_driver->getFrameSize()/2);
            const float texture_size = sin(m_dialog_size*M_PI*0.5f);

            borderArea.UpperLeftCorner.X  =
                center.X + (int)(((int)rect.UpperLeftCorner.X
                                - (int)center.X)*texture_size);
            borderArea.UpperLeftCorner.Y  =
                center.Y + (int)(((int)rect.UpperLeftCorner.Y
                                - (int)center.Y)*texture_size);
            borderArea.LowerRightCorner.X =
                center.X + (int)(((int)rect.LowerRightCorner.X
                                - (int)center.X)*texture_size);
            borderArea.LowerRightCorner.Y =
                center.Y + (int)(((int)rect.LowerRightCorner.Y
                                - (int)center.Y)*texture_size);
        }
        if(widget->m_deactivated)
            GL32_draw2DRectangle(border_color_deactivated, borderArea);
        else if(focused)
            GL32_draw2DRectangle(border_color_focus, borderArea);
        else
            GL32_draw2DRectangle(border_color, borderArea);

        core::recti innerArea = borderArea;
        innerArea.UpperLeftCorner += position2d< s32 >( 3, 3 );
        innerArea.LowerRightCorner -= position2d< s32 >( 3, 3 );
        if(widget->m_deactivated)
            GL32_draw2DRectangle(bg_color_deactivated, innerArea);
        else if(focused)
            GL32_draw2DRectangle(bg_color_focused, innerArea);
        else
            GL32_draw2DRectangle(bg_color, innerArea);
        return;
    }
    else if (type == WTYPE_LIST)
    {
        //drawList(rect, widget, focused);

        drawList(core::recti(widget->m_x, widget->m_y,
                             widget->m_x + widget->m_w,
                             widget->m_y + widget->m_h),
                 widget, focused);
    }
    else if (type == WTYPE_BUBBLE)
    {
        BubbleWidget* bubble = (BubbleWidget*)widget;

        // zoom in/out effect
        if (bubble->isFocusedForPlayer(PLAYER_ID_GAME_MASTER))
        {
            if (bubble->m_zoom < 1.0f)
            {
                bubble->m_zoom += GUIEngine::getLatestDt()*10.0f;
                if (bubble->m_zoom > 1.0f) bubble->m_zoom = 1.0f;

                bubble->updateSize();
            }
        }
        else
        {
            if (bubble->m_zoom > 0.0f)
            {
                bubble->m_zoom -= GUIEngine::getLatestDt()*10.0f;
                if (bubble->m_zoom < 0.0f) bubble->m_zoom = 0.0f;

                bubble->updateSize();
            }
        }

        core::recti rect2 = rect;

        // minor adjustments...
        //rect2.UpperLeftCorner.X -= 7;
        rect2.LowerRightCorner.Y += 7;
        rect2.LowerRightCorner.X += BUBBLE_MARGIN_ON_RIGHT;

        if (bubble->isFocusedForPlayer(PLAYER_ID_GAME_MASTER))
            drawBoxFromStretchableTexture(widget, rect2,
                           SkinConfig::m_render_params["textbubble::focused"]);
        else
            drawBoxFromStretchableTexture(widget, rect2,
                           SkinConfig::m_render_params["textbubble::neutral"]);

        return;
    }
#endif
}   // draw3DSunkenPane

// -----------------------------------------------------------------------------

void Skin::drawBGFadeColor()
{
#ifndef SERVER_ONLY
    // fade out background
    SColor color = SkinConfig::m_colors["dialog_background::neutral"];
    if (m_dialog_size < 1.0f)
        color.setAlpha( (unsigned int)(color.getAlpha()*m_dialog_size ));
    GL32_draw2DRectangle(color, core::recti(position2d< s32 >(0,0),
                         irr_driver->getActualScreenSize()));
#endif
}   // drawBGFadeColor

// -----------------------------------------------------------------------------

core::recti Skin::draw3DWindowBackground(IGUIElement *element,
                                         bool drawTitleBar,
                                         video::SColor titleBarColor,
                                         const core::recti &rect,
                                         const core::recti *clip,
                                         core::recti* checkClientArea)
{
    if (ScreenKeyboard::getCurrent() &&
        ScreenKeyboard::getCurrent()->getIrrlichtElement() == element)
    {
        drawBoxFromStretchableTexture( ScreenKeyboard::getCurrent(), rect,
                           SkinConfig::m_render_params["window::neutral"]);
    }
    else if (ModalDialog::getCurrent() &&
             ModalDialog::getCurrent()->getIrrlichtElement() == element)
    {
        if (ModalDialog::getCurrent()->fadeBackground())
            drawBGFadeColor();
        
        // draw frame
        if (m_dialog_size < 1.0f)
        {
            core::recti sized_rect = rect;
            core::position2d<s32> center = sized_rect.getCenter();
            const int w = sized_rect.getWidth();
            const int h = sized_rect.getHeight();
            const float tex_size = sin(m_dialog_size*M_PI*0.5f);
            sized_rect.UpperLeftCorner.X  = (int)(center.X -(w/2.0f)*tex_size);
            sized_rect.UpperLeftCorner.Y  = (int)(center.Y -(h/2.0f)*tex_size);
            sized_rect.LowerRightCorner.X = (int)(center.X +(w/2.0f)*tex_size);
            sized_rect.LowerRightCorner.Y = (int)(center.Y +(h/2.0f)*tex_size);
            
            drawBoxFromStretchableTexture(ModalDialog::getCurrent(), sized_rect,
                               SkinConfig::m_render_params["window::neutral"]);

            m_dialog_size += GUIEngine::getLatestDt()*5;
        }
        else
        {
            drawBoxFromStretchableTexture(ModalDialog::getCurrent(), rect,
                               SkinConfig::m_render_params["window::neutral"]);
        }
    }

    return rect;
}   // draw3DWindowBackground

// -----------------------------------------------------------------------------

void Skin::draw3DMenuPane (IGUIElement *element, const core::recti &rect,
                           const core::recti *clip)
{
#ifndef SERVER_ONLY
    SColor color = SColor(150, 96, 74, 196);
    GL32_draw2DRectangle(color, rect);
#endif
}   // draw3DMenuPane

// -----------------------------------------------------------------------------

void Skin::draw3DTabBody (IGUIElement *element, bool border, bool background,
                          const core::recti &rect, const core::recti *clip,
                          s32 tabHeight, gui::EGUI_ALIGNMENT alignment)
{
}   // draw3DTabBody

// -----------------------------------------------------------------------------

void Skin::draw3DTabButton (IGUIElement *element, bool active,
                            const core::recti &rect, const core::recti *clip,
                            gui::EGUI_ALIGNMENT alignment)
{
}   // draw3DTabButton

// -----------------------------------------------------------------------------

void Skin::draw3DToolBar (IGUIElement *element, const core::recti &rect,
                          const core::recti *clip)
{
}   // draw3DToolBar

// -----------------------------------------------------------------------------

ITexture* Skin::getImage(const char* name)
{
    if (SkinConfig::m_render_params.find(name)
        != SkinConfig::m_render_params.end())
    {
        BoxRenderParams& p = SkinConfig::m_render_params[name];
        return p.getImage();
    }
    else
    {
        return irr_driver->getTexture(FileManager::GUI,"main_help.png");
    }
}   // getImage

// -----------------------------------------------------------------------------

void Skin::drawIcon (IGUIElement *element, EGUI_DEFAULT_ICON icon,
                     const core::position2di position, u32 starttime,
                     u32 currenttime, bool loop, const core::recti *clip)
{
    // we won't let irrLicht decide when to call this, we draw them ourselves.
    /* m_fallback_skin->drawIcon(element, icon, position, starttime,
                                 currenttime, loop, clip); */
}

// -----------------------------------------------------------------------------

void Skin::draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
    const video::SColor* const colors, bool useAlphaChannelOfTexture)
{
#ifndef SERVER_ONLY
    ::draw2DImage(texture, destRect, sourceRect, clipRect, colors, useAlphaChannelOfTexture);
#endif
}

// -----------------------------------------------------------------------------

video::SColor Skin::getColor (EGUI_DEFAULT_COLOR color) const
{
    /*
     EGDC_3D_DARK_SHADOW    Dark shadow for three-dimensional display elements.
     EGDC_3D_SHADOW     Shadow color for three-dimensional display elements
                        (for edges facing away from the light source).
     EGDC_3D_FACE   Face color for three-dimensional display elements and for
                    dialog box backgrounds.
     EGDC_3D_HIGH_LIGHT     Highlight color for three-dimensional display
                            elements (for edges facing the light source.).
     EGDC_3D_LIGHT  Light color for three-dimensional display elements (for
                    edges facing the light source.).
     EGDC_ACTIVE_BORDER     Active window border.
     EGDC_ACTIVE_CAPTION    Active window title bar text.
     EGDC_APP_WORKSPACE     Background color of multiple document interface
                            (MDI) applications.
     EGDC_BUTTON_TEXT   Text on a button.
     EGDC_GRAY_TEXT     Grayed (disabled) text.
     EGDC_HIGH_LIGHT    Item(s) selected in a control.
     EGDC_HIGH_LIGHT_TEXT   Text of item(s) selected in a control.
     EGDC_INACTIVE_BORDER   Inactive window border.
     EGDC_INACTIVE_CAPTION  Inactive window caption.
     EGDC_TOOLTIP   Tool tip text color.
     EGDC_TOOLTIP_BACKGROUND    Tool tip background color.
     EGDC_SCROLLBAR     Scrollbar gray area.
     EGDC_WINDOW    Window background.
     EGDC_WINDOW_SYMBOL     Window symbols like on close buttons, scroll bars
                             and check boxes.
     EGDC_ICON  Icons in a list or tree.
     EGDC_ICON_HIGH_LIGHT   Selected icons in a list or tree.
     */

    switch(color)
    {
        case EGDC_GRAY_TEXT:
            return SkinConfig::m_colors["text::neutral"];

        case EGDC_HIGH_LIGHT:
        case EGDC_ICON_HIGH_LIGHT:
        case EGDC_HIGH_LIGHT_TEXT:
            return SkinConfig::m_colors["text::focused"];


        case EGDC_BUTTON_TEXT:
        default:
            return SkinConfig::m_colors["text::neutral"];
    }

}   // getColor

// -----------------------------------------------------------------------------

const wchar_t*  Skin::getDefaultText (EGUI_DEFAULT_TEXT text) const
{
    // No idea what this is for
    return L"SuperTuxKart";
}   // getDefaultText

// -----------------------------------------------------------------------------

IGUIFont* Skin::getFont (EGUI_DEFAULT_FONT which) const
{
    return GUIEngine::getFont();
}   // getFont

// -----------------------------------------------------------------------------

u32 Skin::getIcon (EGUI_DEFAULT_ICON icon) const
{
    //return m_fallback_skin->getIcon(icon);
    // return m_fallback_skin->getIcon(irr::gui::EGUI_DEFAULT_ICON);
    return 0;
}

// -----------------------------------------------------------------------------

s32 Skin::getSize (EGUI_DEFAULT_SIZE texture_size) const
{
    return m_fallback_skin->getSize(texture_size);
}

// -----------------------------------------------------------------------------

IGUISpriteBank* Skin::getSpriteBank () const
{
    return m_fallback_skin->getSpriteBank();
}

// -----------------------------------------------------------------------------

void Skin::setColor (EGUI_DEFAULT_COLOR which, video::SColor newColor)
{
    m_fallback_skin->setColor(which, newColor);
}

// -----------------------------------------------------------------------------

void Skin::setDefaultText (EGUI_DEFAULT_TEXT which, const wchar_t *newText)
{
    m_fallback_skin->setDefaultText(which, newText);
}   // setDefaultText

// -----------------------------------------------------------------------------

void Skin::setFont (IGUIFont *font, EGUI_DEFAULT_FONT which)
{
    m_fallback_skin->setFont(font, which);
}   // setFont

// -----------------------------------------------------------------------------

void Skin::setIcon (EGUI_DEFAULT_ICON icon, u32 index)
{
    m_fallback_skin->setIcon(icon, index);
}   // setIcon

// -----------------------------------------------------------------------------

void Skin::setSize (EGUI_DEFAULT_SIZE which, s32 texture_size)
{
    m_fallback_skin->setSize(which, texture_size);
}   // setSize

// -----------------------------------------------------------------------------

void Skin::setSpriteBank (IGUISpriteBank *bank)
{
    m_fallback_skin->setSpriteBank(bank);
}   // setSpriteBank
