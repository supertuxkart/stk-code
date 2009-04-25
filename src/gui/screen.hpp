#ifndef HEADER_SCREEN_HPP
#define HEADER_SCREEN_HPP

#include <map>
#include <string>
#include "ptr_vector.hpp"

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


namespace GUIEngine
{
    class Widget;
   
    void parseScreenFileDiv(irr::io::IrrXMLReader* xml, ptr_vector<Widget>& append_to);
    
    class Screen : public IEventReceiver
    {
        bool m_loaded;
        std::string m_filename;
        ptr_vector<Widget, HOLD> m_widgets;
        void loadFromFile();

        static void addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        void calculateLayout(ptr_vector<Widget>& widgets, Widget* parent=NULL);
    public:
        // current mouse position, read-only...
        int m_mouse_x, m_mouse_y;
        
        Screen(const char* filename);
        bool operator ==(const char* filename) const { return m_filename == filename; }
        
        Widget* getWidget(const char* name);
        Widget* getWidget(const char* name, ptr_vector<Widget>* within_vector);
        Widget* getWidget(const int id, ptr_vector<Widget>* within_vector=NULL);
        
        Widget* getFirstWidget(ptr_vector<Widget>* within_vector=NULL);
        Widget* getLastWidget(ptr_vector<Widget>* within_vector=NULL);
        
        void addWidgets();
        void calculateLayout();
        
        const std::string& getName() const { return m_filename; }
        
        void elementsWereDeleted(ptr_vector<Widget>* within_vector = NULL);
        
        virtual bool OnEvent(const SEvent& event);
        void processAction(const int action, const unsigned int value);

    };
    
}

#endif
