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


#include "config/user_config.hpp"
#include "guiengine/skin.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include <cassert>
#include <iostream>

using namespace GUIEngine;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

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
        
        if(node->get("type", &type) == 0)
        {
            std::cerr << "Error in skin : All elements must have a type\n";
            return;
        }
        node->get("state", &state);
        
        if(node->get("image", &image) == 0)
        {
            std::cerr << "Error in skin : All elements must have an image\n";
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
        
        
        BoxRenderParams newParam;
        newParam.left_border = leftborder;
        newParam.right_border = rightborder;
        newParam.top_border = topborder;
        newParam.bottom_border = bottomborder;
        newParam.hborder_out_portion = hborder_out_portion;
        newParam.vborder_out_portion = vborder_out_portion;
        newParam.preserve_h_aspect_ratios = preserve_h_aspect_ratios;
        
        // call last since it calculates coords considering all other parameters
        newParam.setTexture( GUIEngine::getDriver()->getTexture( (file_manager->getGUIDir() + "/skins/" + image).c_str() ) );
        
        if(areas.size() > 0)
        {
            newParam.areas = 0;
            if(areas.find("body") != std::string::npos) newParam.areas |= BoxRenderParams::BODY;
            if(areas.find("top") != std::string::npos) newParam.areas |= BoxRenderParams::TOP;
            if(areas.find("bottom") != std::string::npos) newParam.areas |= BoxRenderParams::BOTTOM;
            if(areas.find("left") != std::string::npos) newParam.areas |= BoxRenderParams::LEFT;
            if(areas.find("right") != std::string::npos) newParam.areas |= BoxRenderParams::RIGHT;
        }
        
        m_render_params[type+"::"+state] = newParam;
    }
    
    static void parseColor(const XMLNode* node)
    {
        std::string type;
        std::string state = "neutral";
        int r = 0, g = 0, b = 0;
        
        if(node->get("type", &type) == 0)
        {
            std::cerr << "Error in skin : All elements must have a type\n";
            return;
        }
        node->get("state", &state);
        
        node->get("r", &r);
        node->get("g", &g);
        node->get("b", &b);
        
        SColor color = SColor(255, r, g, b);
        
        m_colors[type+"::"+state] = color;
    }
    
    static void loadFromFile(std::string file)
    {
        XMLNode* root = file_manager->createXMLTree(file);
        if(!root)
        {
            std::cerr << "Could not read XML file " << file.c_str() << std::endl;
        }
        
        const int amount = root->getNumNodes();
        for(int i=0; i<amount; i++)
        {
            const XMLNode* node = root->getNode(i);
            
            if(node->getName() == "element")
            {
                parseElement(node);
            }
            else if(node->getName() == "color")
            {
                parseColor(node);
            }
            else
            {
                std::cerr << "Unknown node in XML file : " << node->getName().c_str() << std::endl;
            }
        }// nend for
        
        delete root;
    }
};

#if 0
#pragma mark -
#endif

/** load default values */
BoxRenderParams::BoxRenderParams()
{
    left_border = 0;
    right_border = 0;
    top_border = 0;
    bottom_border = 0;
    preserve_h_aspect_ratios = false;
    
    hborder_out_portion = 0.5;
    vborder_out_portion = 1.0;
    
    areas = BODY | LEFT | RIGHT | TOP | BOTTOM;
    vertical_flip = false;
    y_flip_set = false;
}
void BoxRenderParams::setTexture(ITexture* image)
{
    this->image = image;
    /*
     The source texture is split this way to allow for a stretchable center and borders that don't stretch :
     
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
    
    const int ax = left_border;
    const int ay = top_border;
    const int bx = texture_w - right_border;
    const int by = top_border;
    const int cx = left_border;
    const int cy = texture_h - bottom_border;
    const int dx = texture_w - right_border;
    const int dy = texture_h - bottom_border;
    
    source_area_left = core::rect<s32>(0, ay, cx, cy);
    source_area_center = core::rect<s32>(ax, ay, dx, dy);
    source_area_right = core::rect<s32>(bx, top_border, texture_w, dy);
    
    source_area_top = core::rect<s32>(ax, 0, bx, by);
    source_area_bottom = core::rect<s32>(cx, cy, dx, texture_h);
    
    source_area_top_left = core::rect<s32>(0, 0, ax, ay);
    source_area_top_right = core::rect<s32>(bx, 0, texture_w, top_border);
    source_area_bottom_left = core::rect<s32>(0, cy, cx, texture_h);
    source_area_bottom_right = core::rect<s32>(dx, dy, texture_w, texture_h);    
}
void BoxRenderParams::calculateYFlipIfNeeded()
{
    if(y_flip_set) return;
    
#define FLIP_Y( X ) {     const int y1 = X.UpperLeftCorner.Y; \
const int y2 = X.LowerRightCorner.Y; \
X##_yflip = X; \
X##_yflip.UpperLeftCorner.Y =  y2;\
X##_yflip.LowerRightCorner.Y =  y1;}
    
    FLIP_Y(source_area_left)
    FLIP_Y(source_area_center)
    FLIP_Y(source_area_right)
    
    FLIP_Y(source_area_top)
    FLIP_Y(source_area_bottom)
    
    FLIP_Y(source_area_top_left)
    FLIP_Y(source_area_top_right)
    FLIP_Y(source_area_bottom_left)
    FLIP_Y(source_area_bottom_right)
#undef FLIP_Y
    
    
    y_flip_set = true;
}

#if 0
#pragma mark -
#endif

Skin::Skin(IGUISkin* fallback_skin)
{
    std::string skin_name = file_manager->getGUIDir();
    skin_name += "/skins/";
    skin_name += UserConfigParams::m_skin_file.c_str();
    
    SkinConfig::loadFromFile( skin_name );
    bg_image = NULL;
    
    m_fallback_skin = fallback_skin;
    m_fallback_skin->grab();
    assert(fallback_skin != NULL); 
}

Skin::~Skin()
{
    m_fallback_skin->drop();
}

void Skin::drawBgImage()
{
    
    // ---- background image
    // on one end, making these static is not too clean.
    // on another end, these variables are really only used locally,
    // and making them static avoids doing the same stupid computations every frame
    static core::rect<s32> dest;
    static core::rect<s32> source_area;
    
    if(bg_image == NULL)
    {
        int texture_w, texture_h;
        // TODO/FIXME? - user preferences still include a background image choice 
        bg_image = SkinConfig::m_render_params["background::neutral"].getImage();
        assert(bg_image != NULL);
        texture_w = bg_image->getSize().Width;
        texture_h = bg_image->getSize().Height;
        
        source_area = core::rect<s32>(0, 0, texture_w, texture_h);
        
#ifdef IRR_SVN
        core::dimension2d<u32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
#else
        core::dimension2d<s32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
#endif
        const int screen_w = frame_size.Width;
        const int screen_h = frame_size.Height;
        
        // stretch image vertically to fit
        float ratio = (float)screen_h / texture_h;
        
        // check that with the vertical stretching, it still fits horizontally
        while(texture_w*ratio < screen_w) ratio += 0.1f;
        
        texture_w = (int)(texture_w*ratio);
        texture_h = (int)(texture_h*ratio);
        
        const int clipped_x_space = (texture_w - screen_w);
        
        dest = core::rect<s32>(-clipped_x_space/2, 0, screen_w+clipped_x_space/2, texture_h);
    }
    
    
    GUIEngine::getDriver()->draw2DImage(bg_image, dest, source_area, 0 /* no clipping */, 0, false /* alpha */);
}

void Skin::drawBoxFromStretchableTexture(SkinWidgetContainer* w, const core::rect< s32 > &dest, BoxRenderParams& params)
{
    // check if widget moved. if so, recalculate coords
    if(w->x != dest.UpperLeftCorner.X || w->y != dest.UpperLeftCorner.Y ||
       w->w != dest.getWidth() || w->h != dest.getHeight())
    {
        w->dest_areas_inited = false;
        w->dest_areas_yflip_inited = false;
        w->x = dest.UpperLeftCorner.X;
        w->y = dest.UpperLeftCorner.Y;
        w->w = dest.getWidth();
        w->h = dest.getHeight();
        //std::cout << "widget moved, calculating again\n";
    }

    const ITexture* source = params.getImage();
    const int left_border = params.left_border;
    const int right_border = params.right_border;
    const int top_border = params.top_border;
    const int bottom_border = params.bottom_border;
    const bool preserve_h_aspect_ratios = params.preserve_h_aspect_ratios;
    const float hborder_out_portion = params.hborder_out_portion;
    const float vborder_out_portion = params.vborder_out_portion;
    int areas = params.areas;
    const bool vertical_flip = params.vertical_flip;
    
    const int texture_h = source->getSize().Height;
    
    /*
     The dest area is split this way. Borders can go a bit beyond the given area so
     components inside don't go over the borders
     (how much it exceeds horizontally is specified in 'hborder_out_portion'. vertically is always the totality)
     
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
    
    if(!w->dest_areas_inited)
    {
        w->dest_x = dest.UpperLeftCorner.X;
        w->dest_y = dest.UpperLeftCorner.Y;
        w->dest_x2 = dest.LowerRightCorner.X;
        w->dest_y2 = dest.LowerRightCorner.Y;
        
        //const float xscale = (float)(dest_x2-dest_x)/texture_w;
        const float yscale = (float)(w->dest_y2 - w->dest_y)/texture_h;
        
        int dest_left_border, dest_right_border;
        
        // scale and keep aspect ratio
        if(preserve_h_aspect_ratios)
        {
            dest_left_border   = (int)(left_border * (w->dest_y2 - w->dest_y) / texture_h );
            dest_right_border  = (int)(right_border * (w->dest_y2 - w->dest_y) / texture_h);
        }
        else
        {
            dest_left_border   = (int)(left_border  *std::min<float>(yscale, 1.0));
            dest_right_border  = (int)(right_border *std::min<float>(yscale, 1.0));
        }
        int dest_top_border    = (int)(top_border   *std::min<float>(yscale, 1.0));
        int dest_bottom_border = (int)(bottom_border*std::min<float>(yscale, 1.0));
        
        
        const float hborder_in_portion = 1 - hborder_out_portion;
        const float vborder_in_portion = 1 - vborder_out_portion;
        
        const int ax = (int)(w->dest_x - dest_left_border*hborder_out_portion);
        const int ay = (int)(w->dest_y - dest_top_border*vborder_out_portion);
        
        const int bx = (int)(w->dest_x + dest_left_border*hborder_in_portion);
        const int by = ay;
        
        const int cx = (int)(w->dest_x2 - dest_right_border*hborder_in_portion);
        const int cy = ay;
        
        const int dx = ax;
        const int dy = (int)(w->dest_y + dest_top_border*vborder_in_portion);
        
        const int ex = bx;
        const int ey = dy;
        
        const int fx = cx;
        const int fy = dy;
        
        const int gx = (int)(w->dest_x2 + dest_right_border*hborder_out_portion);
        const int gy = dy;
        
        const int hx = ax;
        const int hy = (int)(w->dest_y2 - dest_bottom_border*vborder_in_portion);
        
        const int ix = bx;
        const int iy = hy;
        
        const int jx = cx;
        const int jy = hy;
        
        const int kx = gx;
        const int ky = hy;
        
        const int lx = bx;
        const int ly = (int)(w->dest_y2 + dest_bottom_border*vborder_out_portion);
        
        const int mx = cx;
        const int my = ly;
        
        const int nx = gx;
        const int ny = ly;
        
        w->dest_area_left         = core::rect<s32>(dx, dy, ix, iy);
        w->dest_area_center       = core::rect<s32>(ex, ey, jx, jy);
        w->dest_area_right        = core::rect<s32>(fx, fy, kx, ky);
        
        w->dest_area_top          = core::rect<s32>(bx, by, fx, fy);
        w->dest_area_bottom       = core::rect<s32>(ix, iy, mx, my);
        
        w->dest_area_top_left     = core::rect<s32>(ax, ay, ex, ey);
        w->dest_area_top_right    = core::rect<s32>(cx, cy, gx, gy);
        w->dest_area_bottom_left  = core::rect<s32>(hx, hy, lx, ly);
        w->dest_area_bottom_right = core::rect<s32>(jx, jy, nx, ny);
        
        w->dest_areas_inited = true;
    }
    
    if(vertical_flip)
    {
        if(!w->dest_areas_yflip_inited)
        {
#define FLIP_Y( X ) {     const int y1 = X.UpperLeftCorner.Y - w->dest_y; \
const int y2 = X.LowerRightCorner.Y - w->dest_y; \
X##_yflip = X; \
X##_yflip.UpperLeftCorner.Y = w->dest_y + (w->dest_y2 - w->dest_y) - y2;\
X##_yflip.LowerRightCorner.Y = w->dest_y + (w->dest_y2 - w->dest_y) - y1;}
        
        FLIP_Y(w->dest_area_left)
        FLIP_Y(w->dest_area_center)
        FLIP_Y(w->dest_area_right)
        
        FLIP_Y(w->dest_area_top)
        FLIP_Y(w->dest_area_bottom)
        
        FLIP_Y(w->dest_area_top_left)
        FLIP_Y(w->dest_area_top_right)
        FLIP_Y(w->dest_area_bottom_left)
        FLIP_Y(w->dest_area_bottom_right)
        
#undef FLIP_Y
        }
        
        w->dest_areas_yflip_inited = true;
        params.calculateYFlipIfNeeded();
    }
    
#define GET_AREA( X ) X = (vertical_flip ? params.X##_yflip : params.X)
    core::rect<s32>& GET_AREA(source_area_left);
    core::rect<s32>& GET_AREA(source_area_center);
    core::rect<s32>& GET_AREA(source_area_right);
    
    core::rect<s32>& GET_AREA(source_area_top);
    core::rect<s32>& GET_AREA(source_area_bottom);
    
    core::rect<s32>& GET_AREA(source_area_top_left);
    core::rect<s32>& GET_AREA(source_area_top_right);
    core::rect<s32>& GET_AREA(source_area_bottom_left);
    core::rect<s32>& GET_AREA(source_area_bottom_right); 
#undef GET_AREA
    
#define GET_AREA( X ) X = (vertical_flip ? w->X##_yflip : w->X)
    core::rect<s32>& GET_AREA(dest_area_left);
    core::rect<s32>& GET_AREA(dest_area_center);
    core::rect<s32>& GET_AREA(dest_area_right);
    
    core::rect<s32>& GET_AREA(dest_area_top);
    core::rect<s32>& GET_AREA(dest_area_bottom);
    
    core::rect<s32>& GET_AREA(dest_area_top_left);
    core::rect<s32>& GET_AREA(dest_area_top_right);
    core::rect<s32>& GET_AREA(dest_area_bottom_left);
    core::rect<s32>& GET_AREA(dest_area_bottom_right);
#undef GET_AREA 
    
    if((areas & BoxRenderParams::LEFT) != 0)
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_left, source_area_left,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    
    if((areas & BoxRenderParams::BODY) != 0)
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_center, source_area_center,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    
    if((areas & BoxRenderParams::RIGHT) != 0)
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_right, source_area_right,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    
    if((areas & BoxRenderParams::TOP) != 0)
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_top, source_area_top,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    if((areas & BoxRenderParams::BOTTOM) != 0)
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_bottom, source_area_bottom,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    
    if( ((areas & BoxRenderParams::LEFT) != 0) && ((areas & BoxRenderParams::TOP) != 0) )
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_top_left, source_area_top_left,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    if( ((areas & BoxRenderParams::RIGHT) != 0) && ((areas & BoxRenderParams::TOP) != 0) )
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_top_right, source_area_top_right,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    if( ((areas & BoxRenderParams::LEFT) != 0) && ((areas & BoxRenderParams::BOTTOM) != 0) )
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_bottom_left, source_area_bottom_left,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    if( ((areas & BoxRenderParams::RIGHT) != 0) && ((areas & BoxRenderParams::BOTTOM) != 0) )
    {
        GUIEngine::getDriver()->draw2DImage(source, dest_area_bottom_right, source_area_bottom_right,
                                            0 /* no clipping */, 0, true /* alpha */);
    }
    
}

void Skin::drawButton(Widget* w, const core::rect< s32 > &rect, const bool pressed, const bool focused)
{
    if(focused)
        drawBoxFromStretchableTexture(w, rect, SkinConfig::m_render_params["button::focused"]);
    else
        drawBoxFromStretchableTexture(w, rect, SkinConfig::m_render_params["button::neutral"]);
}

void Skin::drawRibbon(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused)
{
}

void Skin::drawRibbonChild(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused)
{
    bool mark_selected = widget->isSelected();
    bool always_show_selection = false;
    
    const bool parent_focused = GUIEngine::getGUIEnv()->getFocus() == widget->m_event_handler->m_element;
    
    RibbonType type = ((RibbonWidget*)widget->m_event_handler)->getRibbonType();
    
    /* tab-bar ribbons */
    if(type == RIBBON_TABS)
    {
        BoxRenderParams* params;
        
        if(mark_selected && (focused || parent_focused))
            params = &SkinConfig::m_render_params["tab::focused"];
        else if(mark_selected)
            params = &SkinConfig::m_render_params["tab::down"];
        else
            params = &SkinConfig::m_render_params["tab::neutral"];
        
        
        // automatically guess from position on-screen if tabs go up or down
#ifdef IRR_SVN
        const bool vertical_flip = (unsigned int)rect.UpperLeftCorner.Y < GUIEngine::getDriver()->getCurrentRenderTargetSize().Height/2;
#else
        const bool vertical_flip = rect.UpperLeftCorner.Y < GUIEngine::getDriver()->getCurrentRenderTargetSize().Height/2;
#endif
        params->vertical_flip = vertical_flip;
        
        core::rect< s32 > rect2 = rect;
        if (mark_selected)
        {
            // selected tab should be slighlty bigger than others
            if(vertical_flip)
                rect2.UpperLeftCorner.Y -= 10;
            else
                rect2.LowerRightCorner.Y += 10;
        }
        
        drawBoxFromStretchableTexture(widget, rect2, *params);
        
        
    }
    /* icon ribbons */
    else
    {
        bool use_glow = true;
        
        if (widget->m_event_handler != NULL && widget->m_event_handler->m_properties[PROP_SQUARE] == "true") use_glow = false;
        if (widget->m_event_handler != NULL && widget->m_event_handler->m_event_handler != NULL &&
            widget->m_event_handler->m_event_handler->m_properties[PROP_SQUARE] == "true") use_glow = false;
        
        /* in combo ribbons, always show selection */
        RibbonWidget* w = NULL;
        
        if(widget->m_event_handler != NULL && widget->m_event_handler->m_type == WTYPE_RIBBON)
        {
            w = dynamic_cast<RibbonWidget*>(widget->m_event_handler);
            if(w->getRibbonType() == RIBBON_COMBO) always_show_selection = true;
        }
        
        const bool mark_focused = focused || (parent_focused && w != NULL && w->m_focus == widget) ||
                                  (mark_selected && !always_show_selection && parent_focused);
        
        
        if(always_show_selection && mark_selected)
        {
            core::rect< s32 > rect2 = rect;
            rect2.UpperLeftCorner.X -= rect.getWidth() / 5;
            rect2.UpperLeftCorner.Y -= rect.getHeight() / 5;
            rect2.LowerRightCorner.X += rect.getWidth() / 5;
            rect2.LowerRightCorner.Y += rect.getHeight() / 5;
            
            ITexture* tex_bubble = SkinConfig::m_render_params["selectionHalo::neutral"].getImage();   
            
            const int texture_w = tex_bubble->getSize().Width;
            const int texture_h = tex_bubble->getSize().Height;
            
            core::rect<s32> source_area = core::rect<s32>(0, 0, texture_w, texture_h);
            
            GUIEngine::getDriver()->draw2DImage(tex_bubble, rect2, source_area,
                                                0 /* no clipping */, 0, true /* alpha */);
        }
        
        if(mark_focused)
        {
            if (use_glow)
            {
                int grow = 45;
                static float glow_effect = 0;
                
                
                const float dt = GUIEngine::getLatestDt();
                glow_effect += dt*3;
                if (glow_effect > 6.2832f /* 2*PI */) glow_effect -= 6.2832f;
                grow = (int)(45 + 10*sin(glow_effect));
                
                
                
                const int glow_center_x = rect.UpperLeftCorner.X + rect.getWidth()/2;
                const int glow_center_y = rect.UpperLeftCorner.Y + rect.getHeight() - 5;
                
                ITexture* tex_ficonhighlight = SkinConfig::m_render_params["focusHalo::neutral"].getImage();
                const int texture_w = tex_ficonhighlight->getSize().Width;
                const int texture_h = tex_ficonhighlight->getSize().Height;
                
                core::rect<s32> source_area = core::rect<s32>(0, 0, texture_w, texture_h);
                
                
                const core::rect< s32 > rect2 =  core::rect< s32 >(glow_center_x - 45 - grow,
                                                                   glow_center_y - 25 - grow/2,
                                                                   glow_center_x + 45 + grow,
                                                                   glow_center_y + 25 + grow/2);
                
                GUIEngine::getDriver()->draw2DImage(tex_ficonhighlight, rect2, source_area,
                                                    0 /* no clipping */, 0, true /* alpha */);
            }
            /* if we're not using glow */
            else
            {
                const bool show_focus = focused || parent_focused;
                
                if(!always_show_selection && !show_focus) return;
                
                //const int texture_w = m_tex_squarefocus->getSize().Width;
                //const int texture_h = m_tex_squarefocus->getSize().Height;
                //core::rect<s32> source_area = core::rect<s32>(0, 0, texture_w, texture_h);
                
                drawBoxFromStretchableTexture(w, rect, SkinConfig::m_render_params["squareFocusHalo::neutral"]);
            }
        } // end if mark_focused
        
        
    }
    
}

void Skin::drawSpinnerBody(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused)
{
    if(!focused)
    {
        IGUIElement* focused_widget = GUIEngine::getGUIEnv()->getFocus();
        
        if(focused_widget != NULL && widget->m_children.size()>2)
        {
            if(widget->m_children[0].id == focused_widget->getID() ||
               widget->m_children[1].id == focused_widget->getID() ||
               widget->m_children[2].id == focused_widget->getID())
            {
                focused = true;
            }
        }
    }
    
    
    BoxRenderParams& params = (focused || pressed) ? SkinConfig::m_render_params["spinner::focused"] : 
    SkinConfig::m_render_params["spinner::neutral"];
    drawBoxFromStretchableTexture(widget, rect, params);
    
    const SpinnerWidget* w = dynamic_cast<const SpinnerWidget*>(widget);
    if( w->isGauge() )
    {
        const int handle_size = (int)( widget->h*params.left_border/(float)params.getImage()->getSize().Height );
        const float value = (float)(w->getValue() - w->getMin()) / (w->getMax() - w->getMin());
        
        
        const core::rect< s32 > dest_area = core::rect< s32 >(widget->x + handle_size,
                                                              widget->y,
                                                              widget->x + handle_size + (int)((widget->w - 2*handle_size)*value),
                                                              widget->y + widget->h);
        
        const ITexture* texture = SkinConfig::m_render_params["gaugefill::neutral"].getImage();
        const int texture_w = texture->getSize().Width;
        const int texture_h = texture->getSize().Height;
        
        const core::rect< s32 > source_area = core::rect< s32 >(0, 0, texture_w, texture_h);
        
        GUIEngine::getDriver()->draw2DImage(texture,
                                            dest_area, source_area,
                                            0 /* no clipping */, 0, true /* alpha */);
        
    }
    
}

void Skin::drawSpinnerChild(const core::rect< s32 > &rect, Widget* widget, const bool pressed, bool focused)
{
    if(pressed)
    {
        Widget* spinner = widget->m_event_handler;
        int areas = 0;
        
        //std::cout << "drawing spinner child " << widget->m_properties[PROP_ID].c_str() << std::endl;
        
        if (widget->m_properties[PROP_ID] == "left") areas = BoxRenderParams::LEFT;
        else if (widget->m_properties[PROP_ID] == "right") areas = BoxRenderParams::RIGHT;
        else return;
        
        core::rect< s32 > rect2 = core::rect< s32 >( spinner->x, spinner->y,
                                                    spinner->x + spinner->w,
                                                    spinner->y + spinner->h  );
        
        BoxRenderParams& params = SkinConfig::m_render_params["spinner::down"];
        params.areas = areas;
        drawBoxFromStretchableTexture(widget, rect, params);
    }
    
}

void Skin::drawCheckBox(const core::rect< s32 > &rect, Widget* widget, bool focused)
{ 
    CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);
    
    ITexture* texture;
    
    if(w->getState() == true)
    {
        if(focused)
            texture = SkinConfig::m_render_params["checkbox::focused+checked"].getImage();
        else
            texture = SkinConfig::m_render_params["checkbox::neutral+checked"].getImage();
    }
    else
    {
        if(focused)
            texture = SkinConfig::m_render_params["checkbox::focused+unchecked"].getImage();
        else
            texture = SkinConfig::m_render_params["checkbox::neutral+unchecked"].getImage();
    }
    
    const int texture_w = texture->getSize().Width;
    const int texture_h = texture->getSize().Height;
    
    const core::rect< s32 > source_area = core::rect< s32 >(0, 0, texture_w, texture_h);
    
    GUIEngine::getDriver()->draw2DImage( texture, rect, source_area,
                                        0 /* no clipping */, 0, true /* alpha */);
}


void Skin::drawList(const core::rect< s32 > &rect, Widget* widget, bool focused)
{
    drawBoxFromStretchableTexture(widget, rect, SkinConfig::m_render_params["list::neutral"]);
    
}
void Skin::drawListSelection(const core::rect< s32 > &rect, Widget* widget, bool focused)
{
    ListWidget* list = dynamic_cast<ListWidget*>(widget);
    assert(list != NULL);
    
    if(focused)
        drawBoxFromStretchableTexture(&list->m_selection_skin_info, rect, SkinConfig::m_render_params["listitem::focused"]);
    else
        drawBoxFromStretchableTexture(&list->m_selection_skin_info, rect, SkinConfig::m_render_params["listitem::down"]);
}

/** recusrive function to render all sections (recursion allows to easily travesre the tree of children and sub-children) */
void Skin::renderSections(ptr_vector<Widget>* within_vector)
{
    if(within_vector == NULL) within_vector = &getCurrentScreen()->m_widgets;
    
    const unsigned short widgets_amount = within_vector->size();
    
    for(int n=0; n<widgets_amount; n++)
    {
        Widget& widget = (*within_vector)[n];
        
        if(widget.m_type == WTYPE_DIV)
        {
            if(widget.m_show_bounding_box)
            {
                core::rect< s32 > rect = core::rect<s32>( widget.x, widget.y, widget.x + widget.w, widget.y + widget.h );
                drawBoxFromStretchableTexture(&widget, rect, SkinConfig::m_render_params["section::neutral"]);
            }
            else
            {
                renderSections( &widget.m_children );
            }
        }
    } // next
    
}

#if 0
#pragma mark -
#pragma mark irrlicht skin functions
#endif

void Skin::draw2DRectangle (IGUIElement *element, const video::SColor &color, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    if(GUIEngine::getStateManager()->isGameState()) return; // ignore in game mode
    
    //const bool focused = GUIEngine::getGUIEnv()->hasFocus(element);
    const int id = element->getID();
    
    Widget* widget = GUIEngine::getCurrentScreen()->getWidget(id);
    if(widget == NULL) return;
    
    const bool focused = GUIEngine::getGUIEnv()->hasFocus(element);
    
    const WidgetType type = widget->m_type;
    if(type == WTYPE_LIST)
    {
        // list selection background
        drawListSelection(rect, widget, focused);
    }
    
}
void Skin::process3DPane(IGUIElement *element, const core::rect< s32 > &rect, const bool pressed)
{
    const bool focused = GUIEngine::getGUIEnv()->hasFocus(element);
    
    const int id = element->getID();
    
    Widget* widget = GUIEngine::getCurrentScreen()->getWidget(id);
    
    if(widget == NULL) return;
    
    const WidgetType type = widget->m_type;
    
    // buttons are used for other uses than plain clickable buttons because irrLicht
    // does not have widgets for everything we need. so at render time, we just check
    // which type this button represents and render accordingly
    
    if(widget->m_event_handler != NULL && widget->m_event_handler->m_type == WTYPE_RIBBON)
    {
        drawRibbonChild(rect, widget, pressed /* pressed */, focused /* focused */);
    }
    else if(widget->m_event_handler != NULL && widget->m_event_handler->m_type == WTYPE_SPINNER)
    {
        drawSpinnerChild(rect, widget, pressed /* pressed */, focused /* focused */);
    }
    else if(type == WTYPE_ICON_BUTTON)
    {
        if(!focused) return; /* don't draw any border in this case */
        else drawButton(widget, rect, pressed /* pressed */, true /* focused */);
    }
    else if(type == WTYPE_BUTTON)
    {
        drawButton(widget, rect, pressed, focused);
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
}

void Skin::draw3DButtonPanePressed (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    process3DPane(element, rect, true /* pressed */ );
}

void Skin::draw3DButtonPaneStandard (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    process3DPane(element, rect, false /* pressed */ );
}

void Skin::draw3DSunkenPane (IGUIElement *element, video::SColor bgcolor, bool flat, bool fillBackGround, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    if(element->getType()==gui::EGUIET_EDIT_BOX)
    {
        // TODO : make configurable in skin file
        GUIEngine::getDriver()->draw2DRectangle( SColor(100, 150, 150, 150), rect );
        return;
    }
    
    const int id = element->getID();
    Widget* widget = GUIEngine::getCurrentScreen()->getWidget(id);
    
    if(widget == NULL) return;
    
    const WidgetType type = widget->m_type;
    
    const bool focused = GUIEngine::getGUIEnv()->getFocus() == element;
    
    if(type == WTYPE_LIST)
    {
        drawList(rect, widget, focused);
    }
    
    //if(focused)
    //    GUIEngine::getDriver()->draw2DRectangle( SColor(255, 150, 0, 0), rect );
    //else
    //    GUIEngine::getDriver()->draw2DRectangle( SColor(255, 0, 150, 0), rect );
}

core::rect< s32 > Skin::draw3DWindowBackground (IGUIElement *element, bool drawTitleBar, video::SColor titleBarColor, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    // fade out background
    // TODO : make configurable in skin file
    GUIEngine::getDriver()->draw2DRectangle( SColor(150, 255, 255, 255),
                                            core::rect< s32 >(position2d< s32 >(0,0) , GUIEngine::getDriver()->getCurrentRenderTargetSize()) );
    
    // draw frame (since it's transluscent, draw many times to get opacity)
    // TODO : make configurable in skin file
    drawBoxFromStretchableTexture( ModalDialog::getCurrent(), rect, SkinConfig::m_render_params["window::neutral"]);
    drawBoxFromStretchableTexture( ModalDialog::getCurrent(), rect, SkinConfig::m_render_params["window::neutral"]);
    
    return rect;
}

void Skin::draw3DMenuPane (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    //printf("draw menu pane\n");
}

void Skin::draw3DTabBody (IGUIElement *element, bool border, bool background, const core::rect< s32 > &rect, const core::rect< s32 > *clip, s32 tabHeight, gui::EGUI_ALIGNMENT alignment)
{
    //printf("draw tab body\n");
}

void Skin::draw3DTabButton (IGUIElement *element, bool active, const core::rect< s32 > &rect, const core::rect< s32 > *clip, gui::EGUI_ALIGNMENT alignment)
{
    //printf("draw tab button\n");
}

void Skin::draw3DToolBar (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
}

void Skin::drawIcon (IGUIElement *element, EGUI_DEFAULT_ICON icon, const core::position2di position, u32 starttime, u32 currenttime, bool loop, const core::rect< s32 > *clip)
{
    /* m_fallback_skin->drawIcon(element, icon, position, starttime, currenttime, loop, clip); */
}

video::SColor Skin::getColor (EGUI_DEFAULT_COLOR color) const 
{
    /*
     EGDC_3D_DARK_SHADOW 	Dark shadow for three-dimensional display elements.
     EGDC_3D_SHADOW 	Shadow color for three-dimensional display elements (for edges facing away from the light source).
     EGDC_3D_FACE 	Face color for three-dimensional display elements and for dialog box backgrounds.
     EGDC_3D_HIGH_LIGHT 	Highlight color for three-dimensional display elements (for edges facing the light source.).
     EGDC_3D_LIGHT 	Light color for three-dimensional display elements (for edges facing the light source.).
     EGDC_ACTIVE_BORDER 	Active window border.
     EGDC_ACTIVE_CAPTION 	Active window title bar text.
     EGDC_APP_WORKSPACE 	Background color of multiple document interface (MDI) applications.
     EGDC_BUTTON_TEXT 	Text on a button.
     EGDC_GRAY_TEXT 	Grayed (disabled) text.
     EGDC_HIGH_LIGHT 	Item(s) selected in a control.
     EGDC_HIGH_LIGHT_TEXT 	Text of item(s) selected in a control.
     EGDC_INACTIVE_BORDER 	Inactive window border.
     EGDC_INACTIVE_CAPTION 	Inactive window caption.
     EGDC_TOOLTIP 	Tool tip text color.
     EGDC_TOOLTIP_BACKGROUND 	Tool tip background color.
     EGDC_SCROLLBAR 	Scrollbar gray area.
     EGDC_WINDOW 	Window background.
     EGDC_WINDOW_SYMBOL 	Window symbols like on close buttons, scroll bars and check boxes.
     EGDC_ICON 	Icons in a list or tree.
     EGDC_ICON_HIGH_LIGHT 	Selected icons in a list or tree. 
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
    
}

const wchar_t* 	Skin::getDefaultText (EGUI_DEFAULT_TEXT text) const 
{
    return L"SuperTuxKart";
}

IGUIFont* Skin::getFont (EGUI_DEFAULT_FONT which) const 
{
    return GUIEngine::getFont();
}

u32 Skin::getIcon (EGUI_DEFAULT_ICON icon) const 
{
    //return m_fallback_skin->getIcon(icon);
    // return m_fallback_skin->getIcon(irr::gui::EGUI_DEFAULT_ICON);
    return 0;
}

s32 Skin::getSize (EGUI_DEFAULT_SIZE size) const 
{
    return m_fallback_skin->getSize(size);
}

IGUISpriteBank* Skin::getSpriteBank () const 
{
    return m_fallback_skin->getSpriteBank();
}

//EGUI_SKIN_TYPE 	getType () const

void Skin::setColor (EGUI_DEFAULT_COLOR which, video::SColor newColor)
{
    m_fallback_skin->setColor(which, newColor);
}

void Skin::setDefaultText (EGUI_DEFAULT_TEXT which, const wchar_t *newText)
{
    m_fallback_skin->setDefaultText(which, newText);
}

void Skin::setFont (IGUIFont *font, EGUI_DEFAULT_FONT which)
{
    m_fallback_skin->setFont(font, which);
}

void Skin::setIcon (EGUI_DEFAULT_ICON icon, u32 index)
{
    m_fallback_skin->setIcon(icon, index);
}

void Skin::setSize (EGUI_DEFAULT_SIZE which, s32 size)
{
    m_fallback_skin->setSize(which, size);
    //printf("setting size\n");
}

void Skin::setSpriteBank (IGUISpriteBank *bank)
{
    //printf("setting sprite bank\n");
    m_fallback_skin->setSpriteBank(bank);
    //this->m_bank = bank;
}
