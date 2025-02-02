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

#include "race/tiers_roulette.hpp"
#include "items/powerup.hpp"
#include "network/protocols/server_lobby.hpp"
#include "race/race_manager.hpp"
#include "modes/world.hpp"
#include "utils/string_utils.hpp"
#include <cstddef>
#include <limits>

TierSRoulette* tiers_roulette = nullptr;

#if 0
KartRestrictionMode TierSRoulette::bufToKR(
        const std::size_t size, char* const buf)
{
}  // bufToKR
#endif
// ----------------------------------------------------------------------------
KartRestrictionMode TierSRoulette::bufToKR(
        const std::size_t size, const char* const buf)
{
    if (!strncasecmp(buf, "none", size))
        return KartRestrictionMode::NONE;
    if (!strncasecmp(buf, "kartheavy", size))
        return KartRestrictionMode::HEAVY;
    if (!strncasecmp(buf, "kartmedium", size))
        return KartRestrictionMode::MEDIUM;
    if (!strncasecmp(buf, "kartlight", size))
        return KartRestrictionMode::LIGHT;

    return KartRestrictionMode::NONE;
}  // bufToKR
// ----------------------------------------------------------------------------
Powerup::SpecialModifier TierSRoulette::bufToTSM(
        const std::size_t size, const char* const buf)
{
    if (!strncasecmp(buf, "none", size))        return Powerup::TSM_NONE;
    if (!strncasecmp(buf, "bowlparty", size))   return Powerup::TSM_BOWLPARTY;
    if (!strncasecmp(buf, "cakeparty", size))   return Powerup::TSM_CAKEPARTY;
    if (!strncasecmp(buf, "plungerparty", size))return Powerup::TSM_PLUNGERPARTY;
    if (!strncasecmp(buf, "zipperparty", size)) return Powerup::TSM_ZIPPERPARTY;
    
    return Powerup::TSM_NONE;
}
// ----------------------------------------------------------------------------
// This can be stacked as these are flags
uint32_t TierSRoulette::bufToWTM(const std::size_t size, const char* const buf)
{
    if (!strncasecmp(buf, "itemchaos", size))  return TIERS_TMODIFIER_CHAOSPARTY;
    return 0;
}
// ----------------------------------------------------------------------------
// subbuffer is specified, do not pass buffer with delimiters
TierSRoulette::RouletteEntry::RouletteEntry(
        const std::size_t size, const char* const buf, 
        std::size_t* const out_size)
{
    std::size_t real = TierSRoulette::readOneFromString(
        size, buf, 
        &m_kart_restriction, &m_powerup_special_modifier, &m_world_tmodifiers);
    if (out_size)
        *out_size = real;
}  // RouletteEntry(std::size_t, char*)

std::string TierSRoulette::RouletteEntry::asHumanReadable() const
{
    std::string res;

    switch(m_kart_restriction)
    {
        case KartRestrictionMode::NONE:
            break;
        case KartRestrictionMode::LIGHT:
            res += "LIGHT PARTY (Only light karts)";
            break;
        case KartRestrictionMode::MEDIUM:
            res += "MEDIUM PARTY (Only medium karts)";
            break;
        case KartRestrictionMode::HEAVY:
            res += "HEAVY PARTY (Only heavy karts)";
            break;
    }

    if (!res.empty() &&
            m_powerup_special_modifier != Powerup::SpecialModifier::TSM_NONE)
        res += " + ";
    switch (m_powerup_special_modifier)
    {
        case Powerup::SpecialModifier::TSM_NONE:
            break;
        case Powerup::SpecialModifier::TSM_CAKEPARTY:
            res += "CAKE PARTY (2 cakes per bonus box only)";
            break;
        case Powerup::SpecialModifier::TSM_BOWLPARTY:
            res += "BOWL PARTY (3 bowling balls per bonus box only)";
            break;
    }
    
    if (!res.empty() && m_world_tmodifiers != 0)
        res += " + ";

    if (m_world_tmodifiers & TIERS_TMODIFIER_CHAOSPARTY)
        res += "CHAOS PARTY (10 random powerups, randomized every minute)";

    return res;
}

std::size_t TierSRoulette::readOneFromString(
        std::size_t size, const char* buf,
        KartRestrictionMode* out_kart_restriction,
        Powerup::SpecialModifier* out_powerup_special_modifier,
        uint32_t* out_world_tmodifiers)
{
    *out_kart_restriction = KartRestrictionMode::NONE;
    *out_powerup_special_modifier = Powerup::SpecialModifier::TSM_NONE;
    *out_world_tmodifiers = 0;

    if (!size || !buf)
        return size;

    std::size_t cur_min_id = 0;
    std::size_t cur_max_id = 0;  // for each tested word in the chain
    KartRestrictionMode kart_restriction;
    Powerup::SpecialModifier powerup_special_modifier;
    uint32_t world_tmodifiers;

    do
    {
        for (; cur_max_id < size; cur_max_id++)
        {
            if (buf[cur_max_id] == c_DELIMITER ||
                buf[cur_max_id] == 0)
            {
                size = cur_max_id;
                break;
            }
            if (buf[cur_max_id] == c_UNIFIER)
                break;
        }

        if (cur_max_id > size)
            return size;

        kart_restriction = bufToKR(
                cur_max_id - cur_min_id, buf + cur_min_id);
        powerup_special_modifier = bufToTSM(
                cur_max_id - cur_min_id, buf + cur_min_id);
        world_tmodifiers = bufToWTM(
                cur_max_id - cur_min_id, buf + cur_min_id);

        if (kart_restriction != KartRestrictionMode::NONE)
            *out_kart_restriction = kart_restriction;
        if (powerup_special_modifier != Powerup::SpecialModifier::TSM_NONE)
            *out_powerup_special_modifier = powerup_special_modifier;
        if (world_tmodifiers != 0)
            *out_world_tmodifiers |= world_tmodifiers;

        // why?
        if (cur_max_id == std::numeric_limits<std::size_t>::max())
            return cur_max_id;

        cur_min_id = ++cur_max_id;
    }
    while (cur_max_id < size);

    return cur_max_id;
}

void TierSRoulette::populateFromBuffer(const std::size_t size, const char* buf)
{
    if (!size || !buf)
        return;

    m_roulette_entries.clear();

    std::size_t cur_min_id = 0;
    while (cur_min_id < size)
    {
        RouletteEntry re;
        cur_min_id += readOneFromString(size - cur_min_id, buf + cur_min_id,
               &re.m_kart_restriction, &re.m_powerup_special_modifier, &re.m_world_tmodifiers);
        m_roulette_entries.push_back(
                re);
        Log::verbose("TierSRoulette", "Entry: %s.",
                re.asHumanReadable().c_str());
        if (!cur_min_id)
            return;

    }
}

// Does not update the current iteration.
void TierSRoulette::rotate()
{
    if (m_roulette_entries.size())
        m_current = (m_current + 1) % m_roulette_entries.size();
}

// Update the current situation according the currently selected set
void TierSRoulette::applyChanges(ServerLobby* sl, RaceManager* rm, World* w)
{
    // This can be used later...
    (void)w;

    RouletteEntry& ent = m_roulette_entries[m_current];

    if (rm)
    {
        rm->setPowerupSpecialModifier(ent.m_powerup_special_modifier);
        rm->setWorldTimedModifiers(ent.m_world_tmodifiers);
    }
    if (sl)
    {
        sl->setKartRestrictionMode(ent.m_kart_restriction);
        // Announce the current entry to all the peers
        std::string msg = "Rotating the parties around the roulette! New modifiers has been activated: " +
		ent.asHumanReadable();
	sl->sendStringToAllPeers(msg);
#if 0
        sl->sendStringToAllPeers(msg);
#else
        Log::info("TierSRoulette", "Iterating to %d, current modifiers: %s.",
                m_current, ent.asHumanReadable().c_str());
#endif
    }

}

/* EOF */
