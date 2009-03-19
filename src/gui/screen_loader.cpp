#include "gui/screen.hpp"
#include "gui/engine.hpp"
#include "gui/widget.hpp"
#include <irrlicht.h>
#include <iostream>
#include <irrXML.h>
#include <sstream>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace GUIEngine;

namespace GUIEngine
{
    
void parseScreenFileDiv(irr::io::IrrXMLReader* xml, ptr_vector<Widget>& append_to)
{
    // parse XML file
    while(xml && xml->read())
    {
        
        switch(xml->getNodeType())
        {
            case irr::io::EXN_TEXT:
                break;
            case irr::io::EXN_ELEMENT:
            {
                WidgetType type;
                
                if (!strcmp("div", xml->getNodeName()))
                {
                    type = WTYPE_DIV;
                    append_to.push_back(new Widget());
                }
                else if (!strcmp("ribbon", xml->getNodeName()))
                {
                    type = WTYPE_RIBBON;
                    append_to.push_back(new RibbonWidget());
                }
                else if (!strcmp("buttonbar", xml->getNodeName()))
                {
                    type = WTYPE_RIBBON;
                    append_to.push_back(new RibbonWidget(RIBBON_TOOLBAR));
                }
                else if (!strcmp("tabs", xml->getNodeName()))
                {
                    type = WTYPE_RIBBON;
                    append_to.push_back(new RibbonWidget(RIBBON_TABS));
                }
                else if (!strcmp("spinner", xml->getNodeName()))
                {
                    type = WTYPE_SPINNER;
                    append_to.push_back(new SpinnerWidget());
                }
                else if (!strcmp("button", xml->getNodeName()))
                {
                    type = WTYPE_BUTTON;
                    append_to.push_back(new ButtonWidget());
                }
                else if (!strcmp("gauge", xml->getNodeName()))
                {
                    type = WTYPE_GAUGE;
                    append_to.push_back(new GaugeWidget());
                }
                else if (!strcmp("icon-button", xml->getNodeName()))
                {
                    type = WTYPE_ICON_BUTTON;
                    append_to.push_back(new IconButtonWidget());
                }
                else if (!strcmp("icon", xml->getNodeName()))
                {
                    type = WTYPE_ICON_BUTTON;
                    append_to.push_back(new IconButtonWidget(false));
                }
                else if (!strcmp("checkbox", xml->getNodeName()))
                {
                    type = WTYPE_CHECKBOX;
                    append_to.push_back(new CheckBoxWidget());
                }
                else if (!strcmp("label", xml->getNodeName()))
                {
                    type = WTYPE_LABEL;
                    append_to.push_back(new LabelWidget());
                }
                else if (!strcmp("spacer", xml->getNodeName()))
                {
                    type = WTYPE_NONE;
                    append_to.push_back(new Widget());
                }
                else if (!strcmp("ribbon_grid", xml->getNodeName()))
                {
                    type = WTYPE_RIBBON_GRID;
                    append_to.push_back(new RibbonGridWidget());
                }
                else if (!strcmp("model", xml->getNodeName()))
                {
                    type = WTYPE_MODEL_VIEW;
                    append_to.push_back(new ModelViewWidget());
                    std::cout << "creating a ModelViewWidget\n";
                }
                else
                {
                    std::cerr << "/!\\ Warning /!\\ : unknown tag found in STK GUI file  : '" << xml->getNodeName()  << "'" << std::endl;
                    continue;
                }
                
                Widget& widget = append_to[append_to.size()-1];
                widget.m_type = type;
                
#define READ_PROPERTY( prop_name, prop_flag ) const char* prop_name = xml->getAttributeValue( #prop_name ); \
if(prop_name != NULL) widget.m_properties[prop_flag] = prop_name; else widget.m_properties[prop_flag] = ""
                
                READ_PROPERTY(id,             PROP_ID);
                READ_PROPERTY(proportion,     PROP_PROPORTION);
                READ_PROPERTY(width,          PROP_WIDTH);
                READ_PROPERTY(height,         PROP_HEIGHT);
                READ_PROPERTY(child_width,    PROP_CHILD_WIDTH);
                READ_PROPERTY(child_height,   PROP_CHILD_HEIGHT);
                READ_PROPERTY(word_wrap,      PROP_WORD_WRAP);
                READ_PROPERTY(grow_with_text, PROP_GROW_WITH_TEXT);
                READ_PROPERTY(x,              PROP_X);
                READ_PROPERTY(y,              PROP_Y);
                READ_PROPERTY(layout,         PROP_LAYOUT);
                READ_PROPERTY(align,          PROP_ALIGN);
                READ_PROPERTY(text,           PROP_TEXT);
                READ_PROPERTY(icon,           PROP_ICON);
                READ_PROPERTY(text_align,     PROP_TEXT_ALIGN);
                READ_PROPERTY(min_value,      PROP_MIN_VALUE);
                READ_PROPERTY(max_value,      PROP_MAX_VALUE);
#undef READ_PROPERTY
                
                //std::cout << "loaded widget " << id << std::endl;
                
                // new div, continue parsing with this new div as new parent
                if( widget.m_type == WTYPE_DIV || widget.m_type == WTYPE_RIBBON)
                    parseScreenFileDiv( xml, append_to[append_to.size()-1].m_children );
            }// end case EXN_ELEMENT
                
                break;
            case irr::io::EXN_ELEMENT_END:
            {
                // we're done parsing this 'div', return one step back in the recursive call
                if (!strcmp("div", xml->getNodeName()))
                    return;
                
                // we're done parsing this 'ribbon', return one step back in the recursive call
                if (!strcmp("ribbon", xml->getNodeName()) ||
                    !strcmp("buttonbar", xml->getNodeName()) ||
                    !strcmp("tabs", xml->getNodeName()))
                    return;
            }
                break;
                
            default: break;
        }//end switch
    } // end while
} // end function
    
} // end namespace
