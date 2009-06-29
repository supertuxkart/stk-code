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


#ifndef HEADER_WIDGET_HPP
#define HEADER_WIDGET_HPP

#include <irrlicht.h>
#include <map>

#include "gui/skin.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/vec3.hpp"

using namespace irr;
using namespace gui;

namespace GUIEngine
{
    
    enum WidgetType
    {
        WTYPE_NONE = -1,
        WTYPE_RIBBON,
        WTYPE_SPINNER,
        WTYPE_BUTTON,
        WTYPE_ICON_BUTTON,
        WTYPE_CHECKBOX,
        WTYPE_LABEL,
        WTYPE_SPACER,
        WTYPE_DIV,
        WTYPE_RIBBON_GRID,
        WTYPE_MODEL_VIEW,
        WTYPE_LIST,
        WTYPE_TEXTBOX
    };
    
    enum Property
    {
        PROP_ID = 100,
        PROP_PROPORTION,
        PROP_WIDTH,
        PROP_HEIGHT,
        PROP_CHILD_WIDTH,
        PROP_CHILD_HEIGHT,
        PROP_WORD_WRAP,
        PROP_GROW_WITH_TEXT, // yet unused
        PROP_X,
        PROP_Y,
        PROP_LAYOUT,
        PROP_ALIGN,
        PROP_TEXT,
        PROP_ICON,
        PROP_TEXT_ALIGN,
        PROP_MIN_VALUE,
        PROP_MAX_VALUE,
        PROP_MAX_WIDTH,
        PROP_MAX_HEIGHT,
        PROP_SQUARE
    };
    
    class Widget : public SkinWidgetContainer
    {
    protected:
        friend class RibbonWidget;
        friend class Screen;
        friend class SpinnerWidget;
        friend class Skin;
        friend class RibbonGridWidget;
        
        /**
          * Can be used in children to indicate whether a widget is selected or not
          * - in widgets where it makes sense (e.g. ribbon children) and where the
          * irrLicht widget can not directly contain this state
          */
        bool m_selected;
        
        /**
          * called when left/right keys pressed and focus is on widget. 
          * Returns 'true' if main event handler should be notified of a change.
          * Override in children to be notified of left/right events.
          */
        virtual bool rightPressed() { return false; }
        virtual bool leftPressed() { return false; }
        
        /** used when you set parents - see m_event_handler explainations below.
            returns whether main event callback should be notified or not */
        virtual bool transmitEvent(Widget* w, std::string& originator) { return true; }
        
        /** used when you set eventSupervisors - see m_event_handler explainations below
            called when one of a widget's children is hovered.
            Returns 'true' if main event handler should be notified of a change. */
        virtual bool mouseHovered(Widget* child) { return false; }
        
        /** override in children if you need to know when the widget is focused */
        virtual void focused() {}
        
        void readCoords(Widget* parent=NULL);
        
        /**
          * This is set to NULL by default; set to something else in a widget to mean
          * that events happening on this widget should not go straight into the
          * event handler. Instead, they will first be passed to m_event_handler->transmitEvent,
          * which is usually the parent analysing events from its children.
          * This is especially useful with logical widgets built with more than
          * one irrlicht widgets (e.g. Spinner, Ribbon)
          */
        Widget* m_event_handler;
        
        IGUIElement* m_parent;
        
        static bool convertToCoord(std::string& x, int* absolute, int* percentage);
    public:
        Widget();
        virtual ~Widget() {}
        
        bool m_show_bounding_box;
        
        virtual void update(float delta) { }
        
        /**
         * Create and add the irrLicht widget(s) associated with this object.
         * Call after Widget was read from XML file and laid out.
         */
        virtual void add() {} 
        
        void setParent(IGUIElement* parent);
        
        /**
          * If this widget has any children, they go here. Children can be either
          * specified in the XML file (e.g. Ribbon or Div children), or can also
          * be created automatically for logical widgets built with more than
          * one irrlicht widgets (e.g. Spinner)
          */
        ptr_vector<Widget> m_children;
        
        /** Type of this widget */
        WidgetType m_type;
        
        // FIXME... i forgot the m_ everywhere ... XD
        
        /** coordinates of the widget */
        int x, y, w, h;
        
        /** numerical ID used by irrLicht to identify this widget
          * (not the same as the string identificator specified in the XML file)
          */
        int id;
        
        /**
          * IrrLicht widget created to represent this object.
          */
        IGUIElement* m_element;
        
        /** A map that holds values for all specified widget properties (in the XML file)*/
        std::map<Property, std::string> m_properties;
        
        static void resetIDCounters();
        
        bool isSelected() const { return m_selected; }
    };
    
    class ButtonWidget : public Widget
    {
    public:
        void add();
        virtual ~ButtonWidget() {}
        void setLabel(const char* label);
    };
    
    class LabelWidget : public Widget
    {
    public:
        void add();
        virtual ~LabelWidget() {}
    };
    
    class CheckBoxWidget : public Widget
    {
        bool m_state;
        bool transmitEvent(Widget* w, std::string& originator);
        
    public:
        CheckBoxWidget();
        virtual ~CheckBoxWidget() {}
        
        void add();
        bool getState() const { return m_state; }
        void setState(const bool enabled)  { m_state = enabled; }
    };
    
    
    class SpinnerWidget : public Widget
    {
        int m_value, m_min, m_max;
        std::vector<std::string> m_labels;
        bool m_graphical;
        bool m_gauge;
        
        bool transmitEvent(Widget* w, std::string& originator);
        bool rightPressed();
        bool leftPressed();
    public:
        
        SpinnerWidget(const bool gauge=false);
        virtual ~SpinnerWidget() {}
        
        void setValue(const int new_value);
        void addLabel(std::string label);
        void add();
        bool isGauge()  const { return m_gauge; }
        int  getValue() const { return m_value; }
        int  getMax()   const { return m_max;   }
        int  getMin()   const { return m_min;   }
    };
    
    class IconButtonWidget : public Widget
    {
        bool clickable;
        IGUIStaticText* label;
    public:
        IconButtonWidget(const bool clickable=true);
        virtual ~IconButtonWidget() {}
        
        void add();
        void setLabel(std::string new_label);
    };
    
    enum RibbonType
    {
        RIBBON_COMBO, /* select one item out of many, like in a combo box */
        RIBBON_TOOLBAR, /* a row of individual buttons */
        RIBBON_TABS /* a tab bar */
    };
    
    class RibbonWidget : public Widget
    {
        friend class RibbonGridWidget;
        friend class Screen;
        
        int m_selection;
        RibbonType m_ribbon_type;
        
        void add();
        
        bool rightPressed();
        bool leftPressed();
        bool mouseHovered(Widget* child);
        
        void updateSelection();
        bool transmitEvent(Widget* w, std::string& originator);
        void focused();
        
        ptr_vector<IGUIStaticText, REF> m_labels;
    public:
        Widget* m_focus;
        
        virtual ~RibbonWidget() {}
        
        int getSelection() const { return m_selection; }
        void setSelection(const int i) { m_selection = i; updateSelection(); }
        void select(std::string item);
        
        RibbonType getRibbonType() const { return m_ribbon_type; }
        const std::string& getSelectionIDString() { return m_children[m_selection].m_properties[PROP_ID]; }
        const std::string& getSelectionText() { return m_children[m_selection].m_properties[PROP_TEXT]; }
        void setLabel(const int id, std::string new_name);
        
        RibbonWidget(const RibbonType type=RIBBON_COMBO);
    };
    
    struct ItemDescription
    {
        std::string m_user_name;
        std::string m_code_name;
        std::string m_sshot_file;
    };
    
    class RibbonGridWidget : public Widget
    {
        friend class RibbonWidget;
        
        virtual ~RibbonGridWidget() {}
        
        /* reference pointers only, the actual instances are owned by m_children */
        ptr_vector<RibbonWidget, REF> m_rows;
        
        std::vector<ItemDescription> m_items;
        IGUIStaticText* m_label;
        RibbonWidget* getSelectedRibbon() const;
        RibbonWidget* getRowContaining(Widget* w) const;
        
        void updateLabel(RibbonWidget* from_this_ribbon=NULL);
        
        void propagateSelection();
        void focused();
        
        bool transmitEvent(Widget* w, std::string& originator);
        
        void scroll(const int x_delta);
        
        int m_scroll_offset;
        int m_needed_cols;
        int m_col_amount;
        int m_max_rows;
        bool m_combo;
        
        bool m_has_label;
        
        /* reference pointers only, the actual instances are owned by m_children */
        Widget* m_left_widget;
        Widget* m_right_widget;
    public:
        RibbonGridWidget(const bool combo=false, const int max_rows=4);
        
        void add();
        bool rightPressed();
        bool leftPressed();
        
        void addItem( std::string user_name, std::string code_name, std::string image_file );
        void updateItemDisplay();
        
        bool mouseHovered(Widget* child);
        
        const std::string& getSelectionIDString();
        const std::string& getSelectionText();

        void setSelection(int item_id);
        void setSelection(const std::string& code_name);
    };

    class ModelViewWidget : public Widget
    {
        
        ptr_vector<scene::IMesh, REF> m_models;
        std::vector<Vec3> m_model_location;
        
        video::ITexture* m_texture;
        float angle;
    public:
        ~ModelViewWidget();
        
        void add();
        void addModel(irr::scene::IMesh* mesh, const Vec3& location = Vec3(0,0,0));
        void update(float delta);
    };
    
    class ListWidget : public Widget
    {
    public:
        SkinWidgetContainer m_selection_skin_info;
        
        void add();
        void addItem(const char* item);

        int getSelection() const;
        std::string getSelectionName() const;
        void clear();
    };

    class TextBoxWidget : public Widget
    {
    public:
        void add();
        void addItem(const char* item);
        
        core::stringw getText() const;
    };
    
}
#endif
