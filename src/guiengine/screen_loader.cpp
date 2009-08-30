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


#include "guiengine/screen.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widget.hpp"
#include "utils/translation.hpp"
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

/**
  * Loads a GUI screen from its XML file. Builds a hierarchy of Widget objects whose
  * contents are a direct transcription of the XML file, with little analysis or layout
  * performed on them.
  */
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
                /* find which type of widget is specified by the current tag, and instanciate it */
                if (!strcmp("div", xml->getNodeName()))
                {
                    Widget* w = new Widget();
                    w->m_type = WTYPE_DIV;
                    append_to.push_back(w);
                }
                else if (!strcmp("stkgui", xml->getNodeName()))
                {
                    // outer node that's there only to comply with XML standard (and expat)
                    continue;
                }
                else if (!strcmp("placeholder", xml->getNodeName()))
                {
                    Widget* w = new Widget(true);
                    w->m_type = WTYPE_DIV;
                    append_to.push_back(w);
                }
                else if (!strcmp("box", xml->getNodeName()))
                {
                    Widget* w = new Widget();
                    w->m_type = WTYPE_DIV;
                    w->m_show_bounding_box = true;
                    append_to.push_back(w);
                }
                else if (!strcmp("ribbon", xml->getNodeName()))
                {
                    append_to.push_back(new RibbonWidget());
                }
                else if (!strcmp("buttonbar", xml->getNodeName()))
                {
                    append_to.push_back(new RibbonWidget(RIBBON_TOOLBAR));
                }
                else if (!strcmp("tabs", xml->getNodeName()))
                {
                    append_to.push_back(new RibbonWidget(RIBBON_TABS));
                }
                else if (!strcmp("spinner", xml->getNodeName()))
                {
                    append_to.push_back(new SpinnerWidget());
                }
                else if (!strcmp("button", xml->getNodeName()))
                {
                    append_to.push_back(new ButtonWidget());
                }
                else if (!strcmp("gauge", xml->getNodeName()))
                {
                    append_to.push_back(new SpinnerWidget(true));
                }
                else if (!strcmp("icon-button", xml->getNodeName()))
                {
                    append_to.push_back(new IconButtonWidget());
                }
                else if (!strcmp("icon", xml->getNodeName()))
                {
                    append_to.push_back(new IconButtonWidget(false));
                }
                else if (!strcmp("checkbox", xml->getNodeName()))
                {
                    append_to.push_back(new CheckBoxWidget());
                }
                else if (!strcmp("label", xml->getNodeName()))
                {
                    append_to.push_back(new LabelWidget());
                }
                else if (!strcmp("spacer", xml->getNodeName()))
                {
                    append_to.push_back(new Widget());
                }
                else if (!strcmp("ribbon_grid", xml->getNodeName()))
                {
                    append_to.push_back(new DynamicRibbonWidget());
                }
                else if (!strcmp("scrollable_ribbon", xml->getNodeName()))
                {
                    append_to.push_back(new DynamicRibbonWidget(true, 1));
                }
                else if (!strcmp("scrollable_toolbar", xml->getNodeName()))
                {
                    append_to.push_back(new DynamicRibbonWidget(false, 1));
                }
                else if (!strcmp("model", xml->getNodeName()))
                {
                    append_to.push_back(new ModelViewWidget());
                }
                else if (!strcmp("list", xml->getNodeName()))
                {
                    append_to.push_back(new ListWidget());
                }
                else
                {
                    std::cerr << "/!\\ Warning /!\\ : unknown tag found in STK GUI file  : '" << xml->getNodeName()  << "'" << std::endl;
                    continue;
                }
                
                /* retrieve the created widget */
                Widget& widget = append_to[append_to.size()-1];
                
                /* read widget properties using macro magic */
                
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

                READ_PROPERTY(icon,           PROP_ICON);
                READ_PROPERTY(text_align,     PROP_TEXT_ALIGN);
                READ_PROPERTY(min_value,      PROP_MIN_VALUE);
                READ_PROPERTY(max_value,      PROP_MAX_VALUE);
                READ_PROPERTY(square_items,   PROP_SQUARE);
 
                READ_PROPERTY(max_width,      PROP_MAX_WIDTH);
                READ_PROPERTY(max_height,     PROP_MAX_HEIGHT);
#undef READ_PROPERTY
                                
                const char* text = xml->getAttributeValue( "text" );
                if (text != NULL)
                {
                    /*
                    // FIXME: is this byteswapping necessary on all platforms?
                    stringw byteswap = _(text);
                    const int charcount = byteswap.size();
                    for (int n=0; n<charcount; n++)
                    {
                        short thischar = byteswap[n];                        
                        char* ptr = (char*)&thischar;
                        char byteA = ptr[0];
                        char byteB = ptr[1];
                        ptr[0] = byteB;
                        ptr[1] = byteA;
                        byteswap[n] = thischar;
                    }
                    widget.m_text = byteswap;
                    */
                    widget.m_text = _(text);
                    
                    const wchar_t* original = _(text);
                    
                    std::wcout << L"Output of mbstowcs : " << original << std::endl;
                    
                    stringw debug = original;
                    
                    //std::wcout << L"Original : " << debug.c_str() << std::endl;
                    
                    std::cout << "Before swapping : ";
                    for (int n=0; ; n++)
                    {
                        int thischar = original[n];
                        
                        
                        if (thischar != (int)debug[n])
                        {
                            std::cerr << "(mismatch : " << thischar << " vs " << (short)debug[n] << ")";
                        }
                        
                        if ((thischar & 0xFFFF) == 0)
                        {
                            std::cout << "<EOS>\n";
                            break;
                        }
                        
                        std::cout << (char)(thischar & 0xFF) << "." << (char)((thischar >> 8) & 0xFF) << ".";
                        
                        
                        if (((thischar >> 16) & 0xFFFF) == 0)
                        {
                            std::cout << "<EOS>\n";
                            break;
                        }
                        
                        
                        std::cout << (char)((thischar >> 16) & 0xFF) << "." << (char)((thischar >> 24) & 0xFF) << "/";

                        //char* ptr = (char*)&thischar;
                        //char byteA = ptr[0];
                        //char byteB = ptr[1];
                        
                        //ptr[0] = byteB;
                        //ptr[1] = byteA;
                        //debug[n] = thischar;
                    }
                    std::cout << "\n";
                    //std::wcout << L"Swapped : " << debug.c_str() << std::endl;
                }
                
  /*
                if (widget.m_properties[PROP_TEXT].size() > 0)
                {

                    std::cout << "Raw print : ";
                    for (wchar_t* ptr = utf16; *ptr != 0; ptr++)
                    {
                        std::cout << (*ptr & 0xFF) << " / " << ((*ptr >> 8) & 0xFF) << " / ";
                    }
                    std::cout << "\nIrr strinw print : ";

                    stringc irrstrc = irrstr.c_str();
                    for (u32 n=0; n<irrstrc.size(); n++)
                    {
                        std::cout << irrstrc[n] << " (" << (int)irrstrc[n] << ") ";
                    }
                    std::cout << std::endl;
                    
                    widget.m_properties[PROP_TEXT] = _(widget.m_properties[PROP_TEXT].c_str());
                }*/
                
                /* a new div starts here, continue parsing with this new div as new parent */
                if( widget.m_type == WTYPE_DIV || widget.m_type == WTYPE_RIBBON)
                    parseScreenFileDiv( xml, append_to[append_to.size()-1].m_children );
            }// end case EXN_ELEMENT
                
                break;
            case irr::io::EXN_ELEMENT_END:
            {
                // we're done parsing this 'div', return one step back in the recursive call
                if (!strcmp("div", xml->getNodeName()))
                    return;
                if (!strcmp("box", xml->getNodeName()))
                    return;
                if (!strcmp("placeholder", xml->getNodeName()))
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
