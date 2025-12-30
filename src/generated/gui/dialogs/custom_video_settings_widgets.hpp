// Auto-generated from dialogs/custom_video_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct CustomVideoSettingsWidgets
{
    LabelWidget* title = nullptr;
    CheckBoxWidget* dynamiclight = nullptr;
    SpinnerWidget* shadows = nullptr;
    CheckBoxWidget* mlaa = nullptr;
    CheckBoxWidget* lightscattering = nullptr;
    CheckBoxWidget* glow = nullptr;
    CheckBoxWidget* ibl = nullptr;
    CheckBoxWidget* lightshaft = nullptr;
    CheckBoxWidget* bloom = nullptr;
    CheckBoxWidget* ssao = nullptr;
    CheckBoxWidget* ssr = nullptr;
    CheckBoxWidget* motionblur = nullptr;
    CheckBoxWidget* dof = nullptr;
    CheckBoxWidget* animated_characters = nullptr;
    CheckBoxWidget* texture_compression = nullptr;
    SpinnerWidget* particles_effects = nullptr;
    SpinnerWidget* image_quality = nullptr;
    SpinnerWidget* geometry_detail = nullptr;
    LabelWidget* render_driver_label = nullptr;
    SpinnerWidget* render_driver = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* apply = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        dynamiclight = screen->getWidget<CheckBoxWidget>("dynamiclight");
        shadows = screen->getWidget<SpinnerWidget>("shadows");
        mlaa = screen->getWidget<CheckBoxWidget>("mlaa");
        lightscattering = screen->getWidget<CheckBoxWidget>("lightscattering");
        glow = screen->getWidget<CheckBoxWidget>("glow");
        ibl = screen->getWidget<CheckBoxWidget>("ibl");
        lightshaft = screen->getWidget<CheckBoxWidget>("lightshaft");
        bloom = screen->getWidget<CheckBoxWidget>("bloom");
        ssao = screen->getWidget<CheckBoxWidget>("ssao");
        ssr = screen->getWidget<CheckBoxWidget>("ssr");
        motionblur = screen->getWidget<CheckBoxWidget>("motionblur");
        dof = screen->getWidget<CheckBoxWidget>("dof");
        animated_characters = screen->getWidget<CheckBoxWidget>("animated_characters");
        texture_compression = screen->getWidget<CheckBoxWidget>("texture_compression");
        particles_effects = screen->getWidget<SpinnerWidget>("particles_effects");
        image_quality = screen->getWidget<SpinnerWidget>("image_quality");
        geometry_detail = screen->getWidget<SpinnerWidget>("geometry_detail");
        render_driver_label = screen->getWidget<LabelWidget>("render_driver_label");
        render_driver = screen->getWidget<SpinnerWidget>("render_driver");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        apply = screen->getWidget<IconButtonWidget>("apply");
    }
};

}  // namespace GUIEngine
