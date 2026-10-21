//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.

#ifndef HEADER_CYCLING_INPUT_HPP
#define HEADER_CYCLING_INPUT_HPP

class DeviceManager;

namespace CyclingInput
{
    enum TelemetrySource
    {
        TS_CADENCE = 0,
        TS_WHEEL = 1
    };

    /** Receives the latest value from a platform cycling sensor bridge. */
    void setTelemetry(float sensor_rpm, TelemetrySource source,
                      bool connected);

    /** Returns the calibrated cadence most recently used by the game. */
    float getCadence();

    /** Applies cycling input to the single local player, if available. */
    void update(DeviceManager* device_manager);

    /** Applies the enabled setting to the platform sensor bridge. */
    void applyConfiguration();
}

#endif
