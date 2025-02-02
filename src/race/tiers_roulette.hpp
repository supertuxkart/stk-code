//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#ifndef HEADER_TIERSROULETTE_HPP
#define HEADER_TIERSROULETTE_HPP

#include "network/server_config.hpp"
#include "race/kart_restriction.hpp"
#include <cstddef>
#include <vector>

class ServerLobby;
class RaceManager;
class World;

// ---------------------------------------------------------
// This class works with C strings rather than std::strings,
// the reason being that the files might be read, or any
// other means of std::string being used.
// ---------------------------------------------------------
class TierSRoulette
{
public:

    static const char c_DELIMITER = ',';
    static const char c_UNIFIER = '+';

    struct RouletteEntry
    {
        RouletteEntry()
        {
            m_kart_restriction = KartRestrictionMode::NONE;
            m_powerup_special_modifier = Powerup::SpecialModifier::TSM_NONE;
            m_world_tmodifiers = 0;
        };
        RouletteEntry(std::size_t size, const char* buf,
                std::size_t* out_size = nullptr);

        KartRestrictionMode         m_kart_restriction;
        Powerup::SpecialModifier    m_powerup_special_modifier;
        uint32_t                    m_world_tmodifiers;

        std::string asHumanReadable() const;
    };

private:

    std::vector<struct RouletteEntry>   m_roulette_entries;
    std::size_t                         m_current;

public:
    TierSRoulette() : m_current(0) {};
    ~TierSRoulette() {};

    static std::size_t readOneFromString(
            std::size_t size, const char* buf,
            KartRestrictionMode* out_kart_restriction,
            Powerup::SpecialModifier* out_powerup_special_modifier,
            uint32_t* out_world_tmodifiers);

    // Use this with either ServerConfig, or file reading.
    // Perhaps other things that require C strings, not std::string
    void populateFromBuffer(
            std::size_t size, const char* buf);

    // These do throw an exception once invalid data is encountered

    // Read kart restriction from the C string, return first encountered.
    static KartRestrictionMode bufToKR(std::size_t size, const char* buf);
    // Read TierS' powerup special modifier from the C string, return first encountered.
    static Powerup::SpecialModifier bufToTSM(std::size_t size, const char* buf);
    // Read TierS' world timed modifiers from the C string, return first encountered.
    static uint32_t bufToWTM(std::size_t size, const char* buf);

    // Construct entries from C string
    void readFromString(std::size_t size, const char* buf);

    // Iterate through the entries by one step. Don't forget to apply changes.
    void rotate();

    // Sets the current modifiers into these scopes.
    // Also announces the changes to all peers.
    void applyChanges(ServerLobby* sl, RaceManager* rm, World* w);
};

extern TierSRoulette* tiers_roulette;

#endif

/* EOF */
