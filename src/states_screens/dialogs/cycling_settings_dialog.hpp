//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.

#ifndef HEADER_CYCLING_SETTINGS_DIALOG_HPP
#define HEADER_CYCLING_SETTINGS_DIALOG_HPP

#include "guiengine/modaldialog.hpp"

class CyclingSettingsDialog : public GUIEngine::ModalDialog
{
private:
    void updateValues();
    void updateEnabledState();

public:
    CyclingSettingsDialog(float percent_width, float percent_height);

    virtual void beforeAddingWidgets();
    virtual GUIEngine::EventPropagation processEvent(
        const std::string& event_source);
};

#endif
