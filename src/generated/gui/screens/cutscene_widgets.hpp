// Auto-generated from screens/cutscene.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"

namespace GUIEngine {

struct CutsceneWidgets
{
    ButtonWidget* continue_ = nullptr;

    void bind(Screen* screen)
    {
        continue_ = screen->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
