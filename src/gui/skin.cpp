#include "gui/skin.hpp"
#include "gui/engine.hpp"
#include "gui/screen.hpp"
#include "gui/widget.hpp"
#include <cassert>
#include <iostream>

using namespace GUIEngine;

Skin::Skin(IGUISkin* fallback_skin)
{
    m_fallback_skin = fallback_skin;
    m_fallback_skin->grab();
    assert(fallback_skin != NULL);
}

Skin::~Skin()
{
    m_fallback_skin->drop();
}

/*
 driver->draw2DImage(texture,
 core::rect<s32>(img_x1, img_y1, img_x2, img_y2),
 core::rect<s32>(0,0,texture_size.Width, texture_size.Height) );
 */ 
void Skin::draw2DRectangle (IGUIElement *element, const video::SColor &color, const core::rect< s32 > &pos, const core::rect< s32 > *clip)
{

    // scrollbar backgound
    
    //printf("draw rectangle\n");
    GUIEngine::getDriver()->draw2DRectangle( SColor(255, 0, 150, 150), pos );
}

void Skin::draw3DButtonPanePressed (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    const bool focused = GUIEngine::getGUIEnv()->hasFocus(element);
    
    if(focused)
        GUIEngine::getDriver()->draw2DRectangle( SColor(255, 100, 0, 0), rect );
    else
        GUIEngine::getDriver()->draw2DRectangle( SColor(255, 0, 100, 0), rect );
}

void Skin::draw3DButtonPaneStandard (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
        //printf("draw 3D button pane\n");
    bool focused = GUIEngine::getGUIEnv()->hasFocus(element);
    
    bool draw_border = true;
    bool mark_selected = false;
    
    // buttons are used for other uses than plain clickable buttons because irrLicht
    // does not have widgets for everything we need. so at render time, we just check
    // which type this button represents and set render options accordingly
    if(element->getType() == EGUIET_BUTTON)
    {
        const int id = element->getID();
        if(id != -1)
        {
            Widget* widget = GUIEngine::getCurrentScreen()->getWidget(id);
            
            if(widget != NULL)
            {
                // don't draw border around bitmaps
                if(widget->m_type == WTYPE_ICON_BUTTON && !focused) draw_border = false;
            
                // draw border around ribbon only if focused.
                if(widget->m_type == WTYPE_RIBBON && !focused) draw_border = false;
                
                // check if one of its children has focus (will happen when directly clicking on them)
                if(widget->m_type == WTYPE_RIBBON)
                {
                    const int amount = widget->m_children.size();
                    for(int n=0; n<amount; n++)
                    {
                        if(GUIEngine::getGUIEnv()->hasFocus(widget->m_children[n].m_element))
                        {
                            draw_border = true;
                            focused = true;
                            break;
                        }
                    }
                }
                
                // for ribbon children
                if(widget->isSelected()) mark_selected = true;
                if(widget->m_parent != NULL && widget->m_parent->m_type == WTYPE_RIBBON && 
                   ((RibbonWidget*)widget->m_parent)->getRibbonType() == RIBBON_TOOLBAR &&
                   !GUIEngine::getGUIEnv()->hasFocus(widget->m_parent->m_element)) mark_selected = false;
            }
        }
    }
    
    if(!draw_border && !mark_selected) return;
    
    if(mark_selected)
    {
        const core::rect< s32 > rect2 =  core::rect< s32 >(rect.UpperLeftCorner.X - 5, rect.UpperLeftCorner.Y - 5,
                                                           rect.UpperLeftCorner.X + rect.getWidth() + 5,
                                                           rect.UpperLeftCorner.Y + rect.getHeight() + 5);
        GUIEngine::getDriver()->draw2DRectangle( SColor(2555, 0, 175, 255), rect2 );
    }
    else if(focused)
        GUIEngine::getDriver()->draw2DRectangle( SColor(255, 150, 0, 0), rect );
    else
        GUIEngine::getDriver()->draw2DRectangle( SColor(255, 0, 150, 0), rect );
}

void Skin::draw3DMenuPane (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
        //printf("draw menu pane\n");
}

void Skin::draw3DSunkenPane (IGUIElement *element, video::SColor bgcolor, bool flat, bool fillBackGround, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    // e.g. the checkbox square
        //printf("draw sunken pane\n");
    const bool focused = GUIEngine::getGUIEnv()->getFocus() == element;
    
    if(focused)
        GUIEngine::getDriver()->draw2DRectangle( SColor(255, 150, 0, 0), rect );
    else
        GUIEngine::getDriver()->draw2DRectangle( SColor(255, 0, 150, 0), rect );
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
    //printf("draw toolbar\n");
}

core::rect< s32 > Skin::draw3DWindowBackground (IGUIElement *element, bool drawTitleBar, video::SColor titleBarColor, const core::rect< s32 > &rect, const core::rect< s32 > *clip)
{
    //printf("draw 3d window bg\n");
    return rect;
}

void Skin::drawIcon (IGUIElement *element, EGUI_DEFAULT_ICON icon, const core::position2di position, u32 starttime, u32 currenttime, bool loop, const core::rect< s32 > *clip)
{
    //printf("draw icon\n");
    m_fallback_skin->drawIcon(element, icon, position, starttime, currenttime, loop, clip);
}

video::SColor Skin::getColor (EGUI_DEFAULT_COLOR color) const 
{
    //printf("getting color\n");
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
        case EGDC_BUTTON_TEXT:
            return SColor(255, 0, 0, 0);
            break;
            
        case EGDC_GRAY_TEXT:
            return SColor(255, 80, 80, 80);
            break;
            
        case EGDC_HIGH_LIGHT:
        case EGDC_ICON_HIGH_LIGHT:
        case EGDC_HIGH_LIGHT_TEXT:
            return SColor(255, 0, 150, 0);
            
        default:
            return SColor(255, 255, 255, 255);
    }
    
}

const wchar_t* 	Skin::getDefaultText (EGUI_DEFAULT_TEXT text) const 
{
    //printf("getting default text\n");
    return L"SuperTuxKart";
}

IGUIFont* Skin::getFont (EGUI_DEFAULT_FONT which) const 
{
    //printf("getting font\n");
    return GUIEngine::getFont();
}

u32 Skin::getIcon (EGUI_DEFAULT_ICON icon) const 
{
    //printf("getting icon\n");
    return m_fallback_skin->getIcon(icon);
}

s32 Skin::getSize (EGUI_DEFAULT_SIZE size) const 
{
    //printf("getting size\n");
    return m_fallback_skin->getSize(size);
}

IGUISpriteBank* Skin::getSpriteBank () const 
{
    //printf("getting bank\n");
    return m_fallback_skin->getSpriteBank();
}

//EGUI_SKIN_TYPE 	getType () const

void Skin::setColor (EGUI_DEFAULT_COLOR which, video::SColor newColor)
{
    m_fallback_skin->setColor(which, newColor);
    //printf("setting color\n");
}

void Skin::setDefaultText (EGUI_DEFAULT_TEXT which, const wchar_t *newText)
{
    m_fallback_skin->setDefaultText(which, newText);
    //printf("setting default text\n");
}

void Skin::setFont (IGUIFont *font, EGUI_DEFAULT_FONT which)
{
    m_fallback_skin->setFont(font, which);
    //printf("setting font\n");
}

void Skin::setIcon (EGUI_DEFAULT_ICON icon, u32 index)
{
    m_fallback_skin->setIcon(icon, index);
    //printf("setting icon\n");
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
