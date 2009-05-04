#include "gui/screen.hpp"
#include "gui/engine.hpp"
#include "gui/my_button.hpp"
#include "io/file_manager.hpp"
#include <irrlicht.h>
#include <iostream>
#include <sstream>

#ifndef round
# define round(x)  (floor(x+0.5f))
#endif

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace GUIEngine;

#include "gui/widget.hpp"

static unsigned int id_counter = 0;
static unsigned int id_counter_2 = 1000; // for items that can't be reached with keyboard navigation but can be clicked

// -----------------------------------------------------------------------------
/** When switching to a new screen, this function will be called to reset ID counters
 * (so we start again from ID 0, and don't grow to big numbers) */
void Widget::resetIDCounters()
{
    id_counter = 0;
    id_counter_2 = 1000;
}
// -----------------------------------------------------------------------------
Widget::Widget()
{
    x = -1;
    y = -1;
    w = -1;
    h = -1;
    id = -1;
    m_element = NULL;
    m_type = WTYPE_NONE;
    m_selected = false;
    m_event_handler = NULL;
    m_show_bounding_box = false;
}
// -----------------------------------------------------------------------------
/** 
 * Receives as string the raw property value retrieved from XML file.
 * Will try to make sense of it, as an absolute value or a percentage.
 *
 * Return values :
 *     Will write to either absolute or percentage, depending on the case.
 *     Returns false if couldn't convert to either
 */
bool Widget::convertToCoord(std::string& x, int* absolute /* out */, int* percentage /* out */)
{
    bool is_number;
    int i;
    std::istringstream myStream(x);
    is_number = (myStream >> i)!=0;
    
    if(!is_number) return false;
    
    if( x[x.size()-1] == '%' ) // percentage
    {
        *percentage = i;
        return true;
    }
    else // absolute number
    {
        *absolute = i;
        return true;
    }
}
// -----------------------------------------------------------------------------
/**
 * Finds its x, y, w and h coords from what is specified in the XML properties.
 * Most notably, expands coords relative to parent and percentages.
 */
void Widget::readCoords(Widget* parent)
{
    /* determine widget position and size if not already done by sizers */
    std::string x       = m_properties[PROP_X];
    std::string y       = m_properties[PROP_Y];
    std::string width   = m_properties[PROP_WIDTH];
    std::string height  = m_properties[PROP_HEIGHT];
    
    /* retrieve parent size (or screen size if none). Will be useful for layout
       and especially for percentages. */
    unsigned int parent_w, parent_h, parent_x, parent_y;
    if(parent == NULL)
    {
        core::dimension2d<s32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
        parent_w = frame_size.Width;
        parent_h = frame_size.Height;
        parent_x = 0;
        parent_y = 0;
    }
    else
    {
        parent_w = parent->w;
        parent_h = parent->h;
        parent_x = parent->x;
        parent_y = parent->y;
    }
    
    // ---- try converting to number
    // x coord
    {
        int abs_x = -1, percent_x = -1;
        if(convertToCoord(x, &abs_x, &percent_x ))
        {
            if(abs_x > -1) this->x = parent_x + abs_x;
            else if(abs_x < -1) this->x = parent_x + (parent_w + abs_x);
            else if(percent_x > -1) this->x = parent_x + parent_w*percent_x/100;
        }
    }
    
    // y coord
    {
        int abs_y = -1, percent_y = -1;
        if(convertToCoord(y, &abs_y, &percent_y ))
        {
            if(abs_y > -1) this->y = parent_y + abs_y;
            else if(abs_y < -1) this->y = parent_y + (parent_h + abs_y);
            else if(percent_y > -1) this->y = parent_y + parent_h*percent_y/100;
        }
    }
    
    // ---- if this widget has an icon, get icon size. this can helpful determine its optimal size
    int texture_w = -1, texture_h = -1;
    
    if(m_properties[PROP_ICON].size() > 0)
    {
        ITexture* texture = GUIEngine::getDriver()->getTexture(
                                                               (file_manager->getDataDir() + "/" + m_properties[PROP_ICON]).c_str()
                                                                );
        if(texture != NULL)
        {
            texture_w = texture->getSize().Width;
            texture_h = texture->getSize().Height;
        }
    }
    
    // ---- if this widget has a label, get text length. this can helpful determine its optimal size
    int label_w = -1, label_h = -1;
    if(m_properties[PROP_TEXT].size() > 0)
    {
        IGUIFont* font = GUIEngine::getFont();
        core::dimension2d< s32 > dim = font->getDimension( stringw(m_properties[PROP_TEXT].c_str()).c_str() );
        label_w = dim.Width;
        // FIXME - won't work with multiline labels. thus, for now, when multiple
        // lines are required, we need to specify a height explicitely
        label_h = dim.Height;
    }
        
    // ---- read dimension
    // width
    {
        int abs_w = -1, percent_w = -1;
        if(convertToCoord(width, &abs_w, &percent_w ))
        {
            if(abs_w > -1) this->w = abs_w;
            else if(percent_w > -1) this->w = (int)round(parent_w*percent_w/100.0);
        }
        else if(texture_w > -1) this->w = texture_w;
        else if(label_w > -1) this->w = label_w;
    }
    
    // height
    {
        int abs_h = -1, percent_h = -1;
        if(convertToCoord(height, &abs_h, &percent_h ))
        {
            if(abs_h > -1) this->h = abs_h;
            else if(percent_h > -1) this->h = parent_h*percent_h/100;
        }
        else if(texture_h > -1 && label_h > -1) this->h = texture_h + label_h;
        else if(texture_h > -1) this->h = texture_h;
        else if(label_h > -1) this->h = label_h;
    }
    
    // ---- can't make widget bigger than parent
    if(this->h > (int)parent_h)
    {
        float ratio = (float)parent_h/this->h;

        this->w = (int)(this->w*ratio);
        this->h = (int)(this->h*ratio);
    }
    if(this->w > (int)parent_w)
    {
        float ratio = (float)parent_w/this->w;

        this->w = (int)(this->w*ratio);
        this->h = (int)(this->h*ratio);
    }
    
    // ------ check for given max size
    if(m_properties[PROP_MAX_WIDTH].size() > 0)
    {
        const int max_width = atoi( this->m_properties[PROP_MAX_WIDTH].c_str() );
        if(this->w > max_width) this->w = max_width;
    }
    
    if(m_properties[PROP_MAX_HEIGHT].size() > 0)
    {
        const int max_height = atoi( this->m_properties[PROP_MAX_HEIGHT].c_str() );
        if(this->h > max_height) this->h = max_height;
    }
     
}

#if 0
#pragma mark -
#pragma mark Button Widget
#endif

void ButtonWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    stringw  message = m_properties[PROP_TEXT].c_str();
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, NULL, ++id_counter, message.c_str(), L"");
    
    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
}

#if 0
#pragma mark -
#pragma mark Label Widget
#endif

void LabelWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    const bool word_wrap = m_properties[PROP_WORD_WRAP] == "true";
    stringw  message = m_properties[PROP_TEXT].c_str();
    
    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if(m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if(m_properties[PROP_TEXT_ALIGN] == "right") align = EGUIA_LOWERRIGHT;
    EGUI_ALIGNMENT valign = EGUIA_CENTER ; // TODO - make confiurable through XML file?
    
    IGUIStaticText* irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size, false, word_wrap, NULL, -1);
    m_element = irrwidget;
    irrwidget->setTextAlignment( align, valign );
    
    id = m_element->getID();
    //m_element->setTabOrder(id);
    m_element->setTabStop(false);
    m_element->setTabGroup(false);
}

#if 0
#pragma mark -
#pragma mark Check Box Widget
#endif

CheckBoxWidget::CheckBoxWidget()
{
    m_state = true;
    m_event_handler = this;
}

void CheckBoxWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    stringw message = m_properties[PROP_TEXT].c_str();
    //m_element = GUIEngine::getGUIEnv()->addCheckBox(true /* checked */, widget_size, NULL, ++id_counter, message.c_str());
    
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, NULL, ++id_counter, L"");
    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
}

bool CheckBoxWidget::transmitEvent(Widget* w, std::string& originator)
{
    /* toggle */
    m_state = !m_state;
    
    /* notify main event handler */
    return true;
}

#if 0
#pragma mark -
#pragma mark Icon Button
#endif


// -----------------------------------------------------------------------------
IconButtonWidget::IconButtonWidget(const bool clickable)
{
    IconButtonWidget::clickable = clickable;
    label = NULL;
}
// -----------------------------------------------------------------------------
void IconButtonWidget::add()
{
    ITexture* texture = GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" +m_properties[PROP_ICON]).c_str());
    const int texture_w = texture->getSize().Width, texture_h = texture->getSize().Height;
    /*
    if(w < texture_w) ... ;
    if(h < texture_h) ... ;
     */
    rect<s32> widget_size;    
    if(clickable)
    {
        widget_size = rect<s32>(x, y, x + w, y + h);
        IGUIButton* btn = GUIEngine::getGUIEnv()->addButton(widget_size, NULL, ++id_counter, L"");
        m_element = btn;
        btn->setUseAlphaChannel(true);
        btn->setImage(texture);
        //btn->setDrawBorder(false);
        btn->setTabStop(true);
    }
    else
    {
        // irrlicht widgets don't support scaling while keeping aspect ratio
        // so, happily, let's implement it ourselves
        const int x_gap = (int)((float)w - (float)texture_w * (float)h / texture_h);
        
        widget_size = rect<s32>(x + x_gap/2, y, x + w - x_gap/2, y + h);
        
        IGUIImage* btn = GUIEngine::getGUIEnv()->addImage(widget_size, NULL, ++id_counter_2);
        m_element = btn;
        btn->setUseAlphaChannel(true);
        btn->setImage(texture);
        //btn->setDrawBorder(false);
        btn->setTabStop(false);
        btn->setScaleImage(true);
    }
    stringw  message = m_properties[PROP_TEXT].c_str();
    if(message.size() > 0)
    {
        widget_size += position2d<s32>(0, widget_size.getHeight());
        label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size);
        label->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);
        label->setTabStop(false);
    }
    
    id = m_element->getID();
    if(clickable) m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    
    /*
     IGUISpriteBank* sprite_bank = GUIEngine::getGUIEnv()->getSkin()->getSpriteBank(); 
     // GUIEngine::getDriver()->makeColorKeyTexture(GUIEngine::getDriver()->getTexture("irrlichtlogo2.png"), position2di(0,0)); 
     sprite_bank->addTexture( GUIEngine::getDriver()->getTexture("irrlichtlogo2.png") );
     
     SGUISprite sprite;
     sprite.frameTime = 3000;
     SGUISpriteFrame frame;
     core::array<core::rect<s32> >& rectangles = sprite_bank->getPositions(); 
     rectangles.push_back(rect<s32>(0,0,128,128));
     frame.rectNumber = rectangles.size()-1;
     frame.textureNumber = sprite_bank->getTextureCount() - 1;
     sprite.Frames.push_back(frame);
     sprite_bank->getSprites().push_back(sprite); 
     
     button->setSpriteBank(sprite_bank);
     button->setSprite(EGBS_BUTTON_UP, sprite_bank->getSprites().size()-1);
     button->setSprite(EGBS_BUTTON_DOWN, sprite_bank->getSprites().size()-1);
     */
}

void IconButtonWidget::setLabel(std::string new_label)
{
    std::cout << "trying to set label " << new_label.c_str() << std::endl;

    if(label == NULL) return;
    
    std::cout << "set label " << new_label.c_str() << std::endl;
    
    label->setText( stringw(new_label.c_str()).c_str() );
}

#if 0
#pragma mark -
#pragma mark Ribbon
#endif

RibbonWidget::RibbonWidget(const RibbonType type)
{
    m_selection = 0;
    m_ribbon_type = type;
    updateSelection();
}
// -----------------------------------------------------------------------------
void RibbonWidget::select(std::string item)
{
    const int subbuttons_amount = m_children.size();
    
    for(int i=0; i<subbuttons_amount; i++)
    {  
        if(m_children[i].m_properties[PROP_ID] == item)
        {
            m_selection = i;
            updateSelection();
            return;
        }
    }

}
// -----------------------------------------------------------------------------
bool RibbonWidget::rightPressed()
{
    m_selection++;
    if(m_selection >= m_children.size())
    {
        if(m_event_handler != NULL)
        {
            ((RibbonGridWidget*)m_event_handler)->scroll(1); // FIXME? - find cleaner way to propagate event to parent
            m_selection = m_children.size()-1;
        }
        else m_selection = 0;
    }
    updateSelection();
    
    return m_ribbon_type != RIBBON_TOOLBAR;
}
// -----------------------------------------------------------------------------
bool RibbonWidget::leftPressed()
{
    m_selection--;
    if(m_selection < 0)
    {
        if(m_event_handler != NULL)
        {
            ((RibbonGridWidget*)m_event_handler)->scroll(-1); // FIXME? - find cleaner way to propagate event to parent
            m_selection = 0;
        }
        else m_selection = m_children.size()-1;
    }
    updateSelection();
    
    return m_ribbon_type != RIBBON_TOOLBAR;
}
// -----------------------------------------------------------------------------
void RibbonWidget::focused()
{
    if(m_event_handler != NULL) ((RibbonGridWidget*)m_event_handler)->updateLabel( this );
}
// -----------------------------------------------------------------------------
bool RibbonWidget::mouseHovered(Widget* child)
{
    const int subbuttons_amount = m_children.size();
    
    for(int i=0; i<subbuttons_amount; i++)
    {
        if(m_children.get(i) == child)
        {
            if(m_selection == i) return false; // was already selected, don't send another event
            m_selection = i;
            break;
        }
    }
    updateSelection();
    return m_ribbon_type != RIBBON_TOOLBAR;
}
// -----------------------------------------------------------------------------
void RibbonWidget::updateSelection()
{
    const int subbuttons_amount = m_children.size();
    
    for(int i=0; i<subbuttons_amount; i++)
    {
        m_children[i].m_selected = (i == m_selection);
    }
}
// -----------------------------------------------------------------------------
bool RibbonWidget::transmitEvent(Widget* w, std::string& originator)
{
    const int subbuttons_amount = m_children.size();
    
    for(int i=0; i<subbuttons_amount; i++)
    {
        if(m_children[i].m_properties[PROP_ID] == originator)
        {
            m_selection = i;
            break;
        }
    }
    updateSelection();
    GUIEngine::getGUIEnv()->setFocus(m_element);
    return true;
}
// -----------------------------------------------------------------------------
void RibbonWidget::add()
{
    m_labels.clearWithoutDeleting();

    
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);

    IGUIButton * btn = GUIEngine::getGUIEnv()->addButton(widget_size, NULL, ++id_counter, L"");
    m_element = btn;
    
    const int subbuttons_amount = m_children.size();
    
    // ---- check how much space each child button will take and fit them within available space
    int total_needed_space = 0;
    for(int i=0; i<subbuttons_amount; i++)
    {
        m_children[i].readCoords(this);
        
        if(m_children[i].m_type != WTYPE_ICON_BUTTON && m_children[i].m_type != WTYPE_BUTTON)
        {
            std::cerr << "/!\\ Warning /!\\ : ribbon widgets can only have (icon)button widgets as children " << std::endl;
            continue;
        }
        total_needed_space += m_children[i].w;
    }
 
    int free_h_space = w - total_needed_space;
    
    int biggest_y = 0;
    const int button_y = 10;
    float global_zoom = 1;
    
    const int min_free_space = 50;
    global_zoom = (float)w / (float)( w - free_h_space + min_free_space );
    free_h_space = (int)(w - total_needed_space*global_zoom);
    
    const int one_button_space = (int)round((float)w / (float)subbuttons_amount);
     
    // ---- add children   
    for(int i=0; i<subbuttons_amount; i++)
    {

        const int widget_x = one_button_space*(i+1) - one_button_space/2;
        
        IGUIButton * subbtn;
        
        if(/*m_children[i].m_type == WTYPE_BUTTON*/ getRibbonType() == RIBBON_TABS)
        {
            rect<s32> subsize = rect<s32>(widget_x - one_button_space/2+2,  0, 
                                          widget_x + one_button_space/2-2,  h);
            
            stringw  message = m_children[i].m_properties[PROP_TEXT].c_str();
            
            if(m_children[i].m_type == WTYPE_BUTTON)
            {
                subbtn = GUIEngine::getGUIEnv()->addButton(subsize, btn, ++id_counter_2, message.c_str(), L"");
                subbtn->setTabStop(false);
                subbtn->setTabGroup(false);
            }
            else if(m_children[i].m_type == WTYPE_ICON_BUTTON)
            {
                rect<s32> icon_part = rect<s32>(15,
                                                0,
                                                subsize.getHeight()+15,
                                                subsize.getHeight());
                rect<s32> label_part = rect<s32>(subsize.getHeight()+15,
                                                 0,
                                                 subsize.getWidth()-15,
                                                 subsize.getHeight());                
                
                // use the same ID for all subcomponents; since event handling is done per-ID, no matter
                // which one your hover, this widget will get it
                int same_id = ++id_counter_2;
                subbtn = GUIEngine::getGUIEnv()->addButton(subsize, btn, same_id, L"", L"");
                
                MyGUIButton* icon = new MyGUIButton(GUIEngine::getGUIEnv(), subbtn, same_id, icon_part, true);
                icon->setImage( GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" + m_children[i].m_properties[PROP_ICON]).c_str()) );
                icon->setUseAlphaChannel(true);
                icon->setDrawBorder(false);
                icon->setTabStop(false);
                
                IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), label_part,
                                                                              false /* border */,
                                                                              true /* word wrap */,
                                                                              subbtn, same_id);
                label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
                label->setTabStop(false);
                label->setNotClipped(true);
                m_labels.push_back(label);
                
                subbtn->setTabStop(false);
                subbtn->setTabGroup(false);

            }
   
            m_children[i].m_element = subbtn;
        }
        else if(m_children[i].m_type == WTYPE_ICON_BUTTON)
        {
            const bool has_label = m_children[i].m_properties[PROP_TEXT].size() > 0;
            
            // how much space to keep for the label under the button
            const int needed_space_under_button = has_label ? 30 : 10; // quite arbitrary for now
            // if button too high to fit, scale down
            float zoom = global_zoom;
            while(button_y + m_children[i].h*zoom + needed_space_under_button > h) zoom -= 0.01f;
            
            // ---- add bitmap button part
            const float image_w = m_children[i].w*zoom;
            rect<s32> subsize = rect<s32>(widget_x - (int)(image_w/2.0f), button_y, 
                                          widget_x + (int)(image_w/2.0f), button_y + (int)(m_children[i].h*zoom));
            
            subbtn = new MyGUIButton(GUIEngine::getGUIEnv(), btn, ++id_counter_2, subsize, true);
            
            m_children[i].m_element = subbtn;
            subbtn->setUseAlphaChannel(true);
            subbtn->setImage( GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" + m_children[i].m_properties[PROP_ICON]).c_str()) );

            // ---- label part
            if(has_label)
            {                
                subsize = rect<s32>(widget_x - one_button_space/2,
                                    (int)((button_y + m_children[i].h)*zoom) + 5 /* leave 5 pixels between button and label */, 
                                    widget_x + (int)(one_button_space/2.0f), h);
                
                stringw  message = m_children[i].m_properties[PROP_TEXT].c_str();
                IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), subsize, false, true, btn);
                label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
                label->setTabStop(false);
                label->setNotClipped(true);
                
                m_labels.push_back(label);
                
                const int final_y = subsize.getHeight() + label->getTextHeight();
                if(final_y > biggest_y) biggest_y = final_y;
            }
            
            subbtn->setTabStop(false);
            subbtn->setTabGroup(false);
        }
        else
        {
            std::cerr << "/!\\ Warning /!\\ : Invalid contents type in ribbon" << std::endl;
        }
        
        
        m_children[i].id = subbtn->getID();
        m_children[i].m_event_handler = this;
    }// next sub-button
    
    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    updateSelection();
}

void RibbonWidget::setLabel(const int id, std::string new_name)
{
    if(m_labels.size() == 0) return; // ignore this call for ribbons without labels
    
    assert(id >= 0);
    assert(id < m_labels.size());
    m_labels[id].setText( stringw(new_name.c_str()).c_str() );
}

#if 0
#pragma mark -
#pragma mark Spinner
#endif

SpinnerWidget::SpinnerWidget(const bool gauge)
{
    m_gauge = gauge;
}

void SpinnerWidget::add()
{
    // retrieve min and max values
    std::string min_s = m_properties[PROP_MIN_VALUE];
    std::string max_s = m_properties[PROP_MAX_VALUE];
    
    {
        int i;
        std::istringstream myStream(min_s);
        bool is_number = (myStream >> i)!=0;
        if(is_number) m_min = i;
        else m_min = 0;
    }
    {
        int i;
        std::istringstream myStream(max_s);
        bool is_number = (myStream >> i)!=0;
        if(is_number) m_max = i;
        else m_max = 10;
    }    
    
    m_value = (m_min + m_max)/2;    
    
    // create sub-widgets if they don't already exist
    if(m_children.size() == 0)
    {
        std::string& icon = m_properties[PROP_ICON];
        m_graphical = icon.size()>0;
        
        m_children.push_back( new Widget() );
        m_children.push_back( new Widget() );
        m_children.push_back( new Widget() );
    }
    
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    IGUIButton * btn = GUIEngine::getGUIEnv()->addButton(widget_size, NULL, ++id_counter, L"");
    m_element = btn;
    
    // left arrow
    rect<s32> subsize_left_arrow = rect<s32>(0 ,0, h, h);
    IGUIButton * left_arrow = GUIEngine::getGUIEnv()->addButton(subsize_left_arrow, btn, ++id_counter_2, L" ");
    m_children[0].m_element = left_arrow;
    m_children[0].m_type = WTYPE_BUTTON;
    left_arrow->setTabStop(false);
    m_children[0].m_event_handler = this;
    m_children[0].m_properties[PROP_ID] = "left";
    m_children[0].id = m_children[0].m_element->getID();
    
    // label
    if(m_graphical)
    {
        char imagefile[128];
        std::string icon = file_manager->getDataDir() + "/" + m_properties[PROP_ICON];
        snprintf(imagefile, 128, icon.c_str(), m_value);
        ITexture* texture = GUIEngine::getDriver()->getTexture(imagefile);
        const int texture_width = texture->getSize().Width;
        const int free_h_space = w-h*2-texture_width; // to center image
        
        rect<s32> subsize_label = rect<s32>(h+free_h_space/2, 0, w-h+free_h_space/2, h);
        //IGUIButton* subbtn = GUIEngine::getGUIEnv()->addButton(subsize_label, btn, ++id_counter_2, L"");
        IGUIImage * subbtn = GUIEngine::getGUIEnv()->addImage(subsize_label, btn, ++id_counter_2);
        m_children[1].m_element = subbtn;
        m_children[1].m_type = WTYPE_ICON_BUTTON;
        m_children[1].id = subbtn->getID();
        subbtn->setUseAlphaChannel(true);
        
        subbtn->setImage(texture);
        //subbtn->setScaleImage(true);
    }
    else
    {
        rect<s32> subsize_label = rect<s32>(h, 0, w-h, h);
        IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(stringw(m_value).c_str(), subsize_label,
                                                                      false /* border */, true /* word wrap */,
                                                                      btn, ++id_counter_2);
        m_children[1].m_element = label;
        m_children[1].m_type = WTYPE_LABEL;
        m_children[1].id = label->getID();
        label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
        label->setTabStop(false);
        label->setNotClipped(true);
    }
    
    
    // right arrow
    rect<s32> subsize_right_arrow = rect<s32>(w-h, 0, w, h);
    IGUIButton * right_arrow = GUIEngine::getGUIEnv()->addButton(subsize_right_arrow, btn, ++id_counter_2, L"  ");
    right_arrow->setTabStop(false);
    m_children[2].m_element = right_arrow;
    m_children[2].m_type = WTYPE_BUTTON;
    m_children[2].m_event_handler = this;
    m_children[2].m_properties[PROP_ID] = "right";
    m_children[2].id = m_children[2].m_element->getID();
}
// -----------------------------------------------------------------------------
bool SpinnerWidget::rightPressed()
{
    if(m_value+1 <= m_max) setValue(m_value+1);
    return true;
}
// -----------------------------------------------------------------------------
bool SpinnerWidget::leftPressed()
{
    if(m_value-1 >= m_min) setValue(m_value-1);
    return true;
}
// -----------------------------------------------------------------------------
bool SpinnerWidget::transmitEvent(Widget* w, std::string& originator)
{
    if(originator == "left") leftPressed();
    else if(originator == "right") rightPressed();
    
    GUIEngine::getGUIEnv()->setFocus(m_element);
    return true;
}
// -----------------------------------------------------------------------------
void SpinnerWidget::addLabel(std::string label)
{
    m_labels.push_back(label);
    m_min = 0;
    m_max = m_labels.size()-1;
    setValue(0);
}
// -----------------------------------------------------------------------------
void SpinnerWidget::setValue(const int new_value)
{
    m_value = new_value;
    
    if(m_graphical)
    {
        char imagefile[128];
        std::string icon = file_manager->getDataDir() + "/" + m_properties[PROP_ICON];
        snprintf(imagefile, 128, icon.c_str(), m_value);
        //((IGUIButton*)(m_children[1].m_element))->setImage(GUIEngine::getDriver()->getTexture(imagefile));
        ((IGUIImage*)(m_children[1].m_element))->setImage(GUIEngine::getDriver()->getTexture(imagefile));
    }
    else if(m_labels.size() > 0)
        m_children[1].m_element->setText( stringw(m_labels[new_value].c_str()).c_str() );
    else
        m_children[1].m_element->setText( stringw(m_value).c_str() );
}

#if 0
#pragma mark -
#pragma mark Ribbon Grid Widget
#endif

RibbonGridWidget::RibbonGridWidget(const int max_rows)
{
    m_scroll_offset = 0;
    m_needed_cols = 0;
    m_col_amount = 0;
    m_has_label = false;
    
    m_max_rows = max_rows;
    
    m_left_widget = NULL;
    m_right_widget = NULL;
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::add()
{
    // Work-around for FIXME below... first clear children to avoid duplicates since we're adding everything again everytime
    m_children.clearAndDeleteAll();
    m_rows.clearWithoutDeleting();
    
    m_has_label = m_properties[PROP_TEXT] == "bottom";
    const int label_height = m_has_label ? 25 : 0;
    
    int child_width, child_height;
    child_width = atoi(m_properties[PROP_CHILD_WIDTH].c_str());
    child_height = atoi(m_properties[PROP_CHILD_HEIGHT].c_str());
    
    if( child_width == 0 || child_height == 0 )
    {
        std::cerr << "/!\\ Warning /!\\ : ribbon grid widgets require 'child_width' and 'child_height' arguments" << std::endl;
        child_width = 256;
        child_height = 256;
    }
    
    // decide how many rows and column we can show in the available space
    int row_amount = (int)round((h-label_height) / (float)child_height);
    //if(row_amount < 2) row_amount = 2;
    if(row_amount > m_max_rows) row_amount = m_max_rows;
    
    const float row_height = (float)(h - label_height)/(float)row_amount;
    
    float ratio_zoom = (float)row_height / (float)(child_height - label_height);
    m_col_amount = (int)round( w / ( child_width*ratio_zoom ) );
    
    // std::cout << "w=" << w << " child_width=" << child_width << " ratio_zoom="<< ratio_zoom << " m_col_amount=" << m_col_amount << std::endl;
    
    //if(m_col_amount < 5) m_col_amount = 5;
    
    // add rows
    for(int n=0; n<row_amount; n++)
    {
        RibbonWidget* ribbon;
        if(m_max_rows == 1) // cheap way to detect if it's a regular grid or a scrollable_ribbon. FIXME
            ribbon = new RibbonWidget(RIBBON_COMBO);
        else
            ribbon = new RibbonWidget(RIBBON_TOOLBAR);
        ribbon->x = x;
        ribbon->y = y + (int)(n*row_height);
        ribbon->w = w;
        ribbon->h = (int)(row_height);
        ribbon->m_type = WTYPE_RIBBON;
        ribbon->m_properties[PROP_ID] = this->m_properties[PROP_ID];
        ribbon->m_event_handler = this;
        
        // add columns
        for(int i=0; i<m_col_amount; i++)
        {
            IconButtonWidget* icon = new IconButtonWidget();
            icon->m_properties[PROP_ICON]="gui/track_random.png";
            
            // set size to get proper ratio (as most textures are saved scaled down to 256x256)
            icon->m_properties[PROP_WIDTH] = m_properties[PROP_CHILD_WIDTH];
            icon->m_properties[PROP_HEIGHT] = m_properties[PROP_CHILD_HEIGHT];
            if(m_properties[PROP_TEXT] == "all") icon->m_properties[PROP_TEXT] = "hello";
            
            // std::cout << "ribbon text = " << m_properties[PROP_TEXT].c_str() << std::endl;
            
            icon->m_type = WTYPE_ICON_BUTTON;
            ribbon->m_children.push_back( icon );
        }
        m_children.push_back( ribbon );
        m_rows.push_back( ribbon );
        ribbon->add();
    }
    
    // add label at bottom
    if(m_has_label)
    {
        rect<s32> label_size = rect<s32>(x, y + h - label_height, x+w, y+h);
        m_label = GUIEngine::getGUIEnv()->addStaticText(L"Selecte a track...", label_size, false, true /* word wrap */, NULL, -1);
        m_label->setTextAlignment( EGUIA_CENTER, EGUIA_CENTER );
    }
    
    // add arrow buttons on each side
    // FIXME? these arrow buttons are outside of the widget's boundaries
    if(m_left_widget != NULL)
    {
        // FIXME - do proper memory management, find why it crashes when i try to clean-up
        //delete m_left_widget;
        //delete m_right_widget;
    }
    m_left_widget = new Widget();
    m_right_widget = new Widget();
    
    const int average_y = y + (h-label_height)/2;
    const int button_w = 30, button_h = 50;
    rect<s32> right_arrow_location = rect<s32>(x + w,
                                               average_y - button_h/2,
                                               x + w + button_w,
                                               average_y + button_h/2);
    stringw  rmessage = ">>";
    IGUIButton* right_arrow = GUIEngine::getGUIEnv()->addButton(right_arrow_location, NULL, ++id_counter_2, rmessage.c_str(), L"");
    right_arrow->setTabStop(false);
    m_right_widget->m_element = right_arrow;
    m_right_widget->m_event_handler = this;
    m_right_widget->m_properties[PROP_ID] = "right";
    m_right_widget->id = right_arrow->getID();
    m_children.push_back( m_right_widget );
    
    rect<s32> left_arrow_location = rect<s32>(x - button_w,
                                               average_y - button_h/2,
                                               x,
                                               average_y + button_h/2);
    stringw  lmessage = "<<";
    IGUIButton* left_arrow = GUIEngine::getGUIEnv()->addButton(left_arrow_location, NULL, ++id_counter_2, lmessage.c_str(), L"");
    left_arrow->setTabStop(false);
    m_left_widget->m_element = left_arrow;
    m_left_widget->m_event_handler = this;
    m_left_widget->m_properties[PROP_ID] = "left";
    m_left_widget->id = left_arrow->getID();
    m_children.push_back( m_left_widget );
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::rightPressed()
{
    RibbonWidget* w = getSelectedRibbon();
    if(w != NULL)
    {
        w->rightPressed();
    
        updateLabel();
        propagateSelection();
    }
    return false;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::leftPressed()
{
    RibbonWidget* w = getSelectedRibbon();
    if(w != NULL)
    {
        w->leftPressed();
    
        updateLabel();
        propagateSelection();
    }
    return false;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::transmitEvent(Widget* w, std::string& originator)
{
    if(originator=="left")
    {
        scroll(-1);
        return false;
    }
    if(originator=="right")
    {
        scroll(1);
        return false;
    }
    
    // if it's something else, it might be a ribbon child with its own parent
    if(w->m_event_handler != NULL && w->m_event_handler != this)
        return w->m_event_handler->transmitEvent(w, originator);
    
    // if we got there, must be a ribbon itself. in this case we can just transmit the event directly
    return true;
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::scroll(const int x_delta)
{
    m_scroll_offset += x_delta;
        
    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;
    
    if(m_scroll_offset < 0) m_scroll_offset = max_scroll;
    else if(m_scroll_offset > max_scroll) m_scroll_offset = 0;
    
    updateItemDisplay();
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::mouseHovered(Widget* child)
{
    updateLabel();
    propagateSelection();
    return false;
}
// -----------------------------------------------------------------------------
/** RibbonGridWidget is made of several ribbons; each of them thus has
 its own selection independently of each other. To keep a grid feeling
 (i.e. you remain in the same column when pressing up/down), this method is
 used to ensure that all children ribbons always select the same column */
void RibbonGridWidget::propagateSelection()
{
    // find selection in current ribbon
    RibbonWidget* selected_ribbon = (RibbonWidget*)getSelectedRibbon();
    if(selected_ribbon == NULL) return;
    const int i = selected_ribbon->m_selection;
    
    // set same selection in all ribbons
    const int row_amount = m_rows.size();
    for(int n=0; n<row_amount; n++)
    {
        RibbonWidget* ribbon = m_rows.get(n);
        if(ribbon != selected_ribbon)
        {
            ribbon->m_selection = i;
            ribbon->updateSelection();
        }
    }
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::focused()
{
    updateLabel();
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::updateLabel(RibbonWidget* from_this_ribbon)
{
    if(!m_has_label) return;
    
    RibbonWidget* row = from_this_ribbon ? from_this_ribbon : (RibbonWidget*)getSelectedRibbon();
    if(row == NULL) return;
    
    
    std::string selection_id = row->getSelectionName();
    
    const int amount = m_items.size();
    for(int n=0; n<amount; n++)
    {
        if(m_items[n].m_code_name == selection_id)
        {
            m_label->setText( stringw(m_items[n].m_user_name.c_str()).c_str() );
            return;
        }
    }
    
    m_label->setText( L"Random" );
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::addItem( std::string user_name, std::string code_name, std::string image_file )
{
    ItemDescription desc;
    desc.m_user_name = user_name;
    desc.m_code_name = code_name;
    desc.m_sshot_file = image_file;
    
    m_items.push_back(desc);
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::updateItemDisplay()
{
    int icon_id = 0;
    
    const int row_amount = m_rows.size();
    const int item_amount = m_items.size();
    
    m_needed_cols = (int)ceil( (float)item_amount / (float)row_amount );

    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;
    
    for(int n=0; n<row_amount; n++)
    {
        RibbonWidget& row = m_rows[n];
        
        for(int i=0; i<m_col_amount; i++)
        {
            IconButtonWidget* icon = dynamic_cast<IconButtonWidget*>(&row.m_children[i]);
            assert(icon != NULL);
            IGUIButton* button = dynamic_cast<IGUIButton*>(icon->m_element);
            assert(button != NULL);
            
            int col_scroll = i + m_scroll_offset;
            while(col_scroll > max_scroll) col_scroll -= max_scroll+1;
            
            icon_id = (col_scroll)*row_amount + n;

            if( icon_id < item_amount )
            {
                std::string track_sshot = file_manager->getDataDir() + "/" + m_items[icon_id].m_sshot_file;
                button->setImage( GUIEngine::getDriver()->getTexture(  track_sshot.c_str() ));
                button->setPressedImage( GUIEngine::getDriver()->getTexture( track_sshot.c_str()) );
                icon->m_properties[PROP_ID] = m_items[icon_id].m_code_name;
                row.setLabel(i, m_items[icon_id].m_user_name);
            }
            else
            {
                button->setImage( GUIEngine::getDriver()->getTexture( (file_manager->getGUIDir() + "/track_random.png").c_str() ) );
                button->setPressedImage( GUIEngine::getDriver()->getTexture( (file_manager->getGUIDir() + "/track_random.png").c_str() ) );
                icon->m_properties[PROP_ID] = "gui/track_random.png";
            }
        } // next column
    } // next row
}
// -----------------------------------------------------------------------------
const std::string& RibbonGridWidget::getSelectionName()
{
    RibbonWidget* row = (RibbonWidget*)getSelectedRibbon();
    if(row != NULL) return row->getSelectionName();
    
    static const std::string nothing = "";
    return nothing;
}
// -----------------------------------------------------------------------------
RibbonWidget* RibbonGridWidget::getRowContaining(Widget* w) const
{
    const int row_amount = m_rows.size();
    for(int n=0; n<row_amount; n++)
    {
        const RibbonWidget* row = &m_rows[n];
        if(row != NULL)
        {
            if(m_children.contains( w ) ) return (RibbonWidget*)row;
        }
    }
    
    return NULL;
}
// -----------------------------------------------------------------------------
RibbonWidget* RibbonGridWidget::getSelectedRibbon() const
{
    const int row_amount = m_rows.size();
    for(int n=0; n<row_amount; n++)
    {
        const RibbonWidget* row = &m_rows[n];
        if(row != NULL)
        {
            if( GUIEngine::getGUIEnv()->hasFocus(row->m_element) ||
                m_element->isMyChild( GUIEngine::getGUIEnv()->getFocus() ) ) return (RibbonWidget*)row;
        }
    }
    
    return NULL;
}

#if 0
#pragma mark -
#pragma mark Model View Widget
#endif

// -----------------------------------------------------------------------------
void ModelViewWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    stringw  message = m_properties[PROP_TEXT].c_str();
    
    m_element = GUIEngine::getGUIEnv()->addMeshViewer(widget_size, NULL, ++id_counter_2);
    
    id = m_element->getID();
    //m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    m_element->setTabStop(false);
}
// -----------------------------------------------------------------------------
void ModelViewWidget::setModel(SAnimatedMesh* mesh)
{
    ((IGUIMeshViewer*)m_element)->setMesh( mesh );
    
    video::SMaterial mat = mesh->getMeshBuffer(0)->getMaterial(); //mesh_view->getMaterial();
    mat.setFlag(EMF_LIGHTING , false);
    //mat.setFlag(EMF_GOURAUD_SHADING, false);
    //mat.setFlag(EMF_NORMALIZE_NORMALS, true);
    ((IGUIMeshViewer*)m_element)->setMaterial(mat);
}

#if 0
#pragma mark -
#pragma mark List Widget
#endif


// -----------------------------------------------------------------------------
void ListWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);

    IGUIListBox* list = GUIEngine::getGUIEnv()->addListBox (widget_size, NULL, ++id_counter);
    
    id = list->getID();
    list->addItem( L"Hiker" );
    list->addItem( L"Conso" );
    list->addItem( L"Auria" );
    list->addItem( L"MiniBjorn" );
    list->addItem( L"Arthur" );
    
    m_element = list;
    
    //list->setSelected(0);
}
