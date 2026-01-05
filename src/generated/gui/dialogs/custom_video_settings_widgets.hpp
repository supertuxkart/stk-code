// Auto-generated from dialogs/custom_video_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        dynamiclight = container->getWidget<CheckBoxWidget>("dynamiclight");
        shadows = container->getWidget<SpinnerWidget>("shadows");
        mlaa = container->getWidget<CheckBoxWidget>("mlaa");
        lightscattering = container->getWidget<CheckBoxWidget>("lightscattering");
        glow = container->getWidget<CheckBoxWidget>("glow");
        ibl = container->getWidget<CheckBoxWidget>("ibl");
        lightshaft = container->getWidget<CheckBoxWidget>("lightshaft");
        bloom = container->getWidget<CheckBoxWidget>("bloom");
        ssao = container->getWidget<CheckBoxWidget>("ssao");
        ssr = container->getWidget<CheckBoxWidget>("ssr");
        motionblur = container->getWidget<CheckBoxWidget>("motionblur");
        dof = container->getWidget<CheckBoxWidget>("dof");
        animated_characters = container->getWidget<CheckBoxWidget>("animated_characters");
        texture_compression = container->getWidget<CheckBoxWidget>("texture_compression");
        particles_effects = container->getWidget<SpinnerWidget>("particles_effects");
        image_quality = container->getWidget<SpinnerWidget>("image_quality");
        geometry_detail = container->getWidget<SpinnerWidget>("geometry_detail");
        render_driver_label = container->getWidget<LabelWidget>("render_driver_label");
        render_driver = container->getWidget<SpinnerWidget>("render_driver");
        buttons = container->getWidget<RibbonWidget>("buttons");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        apply = container->getWidget<IconButtonWidget>("apply");
    }
};

}  // namespace GUIEngine
