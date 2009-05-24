#ifndef __HEADER_OPTIONS_SCREEN_HPP__
#define __HEADER_OPTIONS_SCREEN_HPP__

#include <string>

namespace GUIEngine
{
    class Widget;
}

namespace StateManager
{

    void menuEventOptions(GUIEngine::Widget* widget, const std::string& name);

}

#endif
