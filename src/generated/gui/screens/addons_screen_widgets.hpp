// Auto-generated from screens/addons_screen.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct AddonsScreenWidgets
{
    IconButtonWidget* back = nullptr;
    IconButtonWidget* reload = nullptr;
    TextBoxWidget* filter_name = nullptr;
    SpinnerWidget* filter_date = nullptr;
    SpinnerWidget* filter_rating = nullptr;
    IconButtonWidget* filter_search = nullptr;
    RibbonWidget* category = nullptr;
    IconButtonWidget* tab_kart = nullptr;
    IconButtonWidget* tab_track = nullptr;
    IconButtonWidget* tab_arena = nullptr;
    SpinnerWidget* filter_installation = nullptr;
    SpinnerWidget* filter_featured = nullptr;
    ListWidget* list_addons = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        reload = container->getWidget<IconButtonWidget>("reload");
        filter_name = container->getWidget<TextBoxWidget>("filter_name");
        filter_date = container->getWidget<SpinnerWidget>("filter_date");
        filter_rating = container->getWidget<SpinnerWidget>("filter_rating");
        filter_search = container->getWidget<IconButtonWidget>("filter_search");
        category = container->getWidget<RibbonWidget>("category");
        tab_kart = container->getWidget<IconButtonWidget>("tab_kart");
        tab_track = container->getWidget<IconButtonWidget>("tab_track");
        tab_arena = container->getWidget<IconButtonWidget>("tab_arena");
        filter_installation = container->getWidget<SpinnerWidget>("filter_installation");
        filter_featured = container->getWidget<SpinnerWidget>("filter_featured");
        list_addons = container->getWidget<ListWidget>("list_addons");
    }
};

}  // namespace GUIEngine
