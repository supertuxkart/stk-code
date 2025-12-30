// Auto-generated from screens/grand_prix_editor.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct GrandPrixEditorWidgets
{
    IconButtonWidget* back = nullptr;
    DynamicRibbonWidget* gplist = nullptr;
    RibbonWidget* gpgroups = nullptr;
    LabelWidget* gpname = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* menu = nullptr;
    IconButtonWidget* new_ = nullptr;
    IconButtonWidget* copy = nullptr;
    IconButtonWidget* edit = nullptr;
    IconButtonWidget* remove = nullptr;
    IconButtonWidget* rename = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        gplist = container->getWidget<DynamicRibbonWidget>("gplist");
        gpgroups = container->getWidget<RibbonWidget>("gpgroups");
        gpname = container->getWidget<LabelWidget>("gpname");
        tracks = container->getWidget<DynamicRibbonWidget>("tracks");
        menu = container->getWidget<RibbonWidget>("menu");
        new_ = container->getWidget<IconButtonWidget>("new");
        copy = container->getWidget<IconButtonWidget>("copy");
        edit = container->getWidget<IconButtonWidget>("edit");
        remove = container->getWidget<IconButtonWidget>("remove");
        rename = container->getWidget<IconButtonWidget>("rename");
    }
};

}  // namespace GUIEngine
