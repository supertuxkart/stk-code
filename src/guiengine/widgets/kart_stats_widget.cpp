//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/kart_stats_widget.hpp"
#include "karts/abstract_characteristic.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "io/file_manager.hpp"

#include <IGUIEnvironment.h>
#include <IGUIElement.h>
#include <IGUIButton.h>
#include <string>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------
KartStatsWidget::KartStatsWidget(core::recti area, const int player_id,
                                 std::string kart_group, bool multiplayer,
                                 bool display_icons) : Widget(WTYPE_DIV)
{
    m_title_font = !multiplayer;
    m_player_id = player_id;

    const std::string default_kart = UserConfigParams::m_default_kart;
    const KartProperties* props =
        kart_properties_manager->getKart(default_kart);
    if(!props)
    {
        // If the default kart can't be found (e.g. previously a addon
        // kart was used, but the addon package was removed), use the
        // first kart as a default. This way we don't have to hardcode
        // any kart names.
        int id = kart_properties_manager->getKartByGroup(kart_group, 0);
        if (id == -1)
            props = kart_properties_manager->getKartById(0);
        else
            props = kart_properties_manager->getKartById(id);

        if(!props)
            Log::fatal("KartSelectionScreen", "Can't find default "
                       "kart '%s' nor any other kart.",
                       default_kart.c_str());
    }
                                                                                                                                    
    for (int i = 0; i < SKILL_COUNT; ++i)
    {
        irr::core::recti skillArea(0, 0, 1, 1);

        SkillLevelWidget* skill_bar = NULL;

        skill_bar = new SkillLevelWidget(skillArea, m_player_id, multiplayer, display_icons);       

        m_skills.push_back(skill_bar);
        m_children.push_back(skill_bar);
    }

	setValues(props, HANDICAP_NONE);

    move(area.UpperLeftCorner.X, area.UpperLeftCorner.Y,
         area.getWidth(), area.getHeight());
}   // KartStatsWidget

// -----------------------------------------------------------------------------
void KartStatsWidget::setValues(const KartProperties* props, HandicapLevel h)
{
    // If props is NULL (e.g., when kart selection is "?" (random)),
    // skip this function to avoid errors.
    if( props == NULL ) return;

    float skill_pts = 0;
	
    // Use kart properties computed for best difficulty to show the user, so
    // that properties don't change according to the the last used difficulty
    RaceManager::Difficulty previous_difficulty = RaceManager::get()->getDifficulty();
    RaceManager::get()->setDifficulty(RaceManager::DIFFICULTY_BEST);
    KartProperties kp_computed;
    kp_computed.copyForPlayer(props, h);
    const AbstractCharacteristic* kpc = kp_computed.getCombinedCharacteristic();
    for (SkillLevelWidget* skills : m_skills)
        skills->setVisible(true);

    // Scale the values so they look better
    // A value of 100 takes the whole bar width, including borders.
    // So values should be in the 0-99 range
    // TODO: pull some parameters from a config file, to allow mods that change
    //       various gameplay value to still offer a working skill display.

    // We use a logarithm so that a given width corresponds to a given weight ratio
    // instead of a given absolute weight difference. This allows for a more meaningful
    // display, as the gameplay effects are proportional to the weight ratio.
    float mass_log = std::logf(kpc->getMass());
    float mass_display = (mass_log * 109.0f) - 545.0f;
    skill_pts += setSkillValues(SKILL_MASS, mass_display, "mass.png", "mass", _("Mass"));
    
    // The base speed is of 28
    // Speed is the characteristic most affected by handicap, but it is also
    // important to display the base differences between classes as significant,
    // as small differences matter a lot.
    // Therefore, a non-linear formula is used.
    float speed_power_four = kpc->getEngineMaxSpeed();
    speed_power_four *= speed_power_four; // squaring
    speed_power_four *= speed_power_four; // squaring again
    speed_power_four *= 0.001f; // divide by 1000 to improve readbility of the formula
    float speed_composite = (speed_power_four - 100.0f) * 0.1924f;
    skill_pts += setSkillValues(SKILL_SPEED, speed_composite,
                   "speed.png", "speed", _("Maximum speed"));
    
    // The acceleration depend on power and mass, and it changes depending on speed
    // We call a function which gives us a single number to represent it
    // power/mass gives numbers in the 1-11 range, so we multiply it by 9.
    skill_pts += setSkillValues(SKILL_ACCELERATION,
                   kp_computed.getAccelerationEfficiency()*9.0f,
                   "power.png", "acceleration", _("Acceleration"));

    // The base nitro consumption is 1.17, lower for lighter karts.
    // The base max speed increase is 5, higher for lighter karts
    float efficiency = 7.0f * kpc->getNitroMaxSpeedIncrease()
                            / kpc->getNitroConsumption();
    // We also take into account the ratio between the minimum burst duration
    // and the nitro bonus time from a minimum burst (with the extra duration)
    efficiency *= (kpc->getNitroMinBurst() + kpc->getNitroDuration())
                  / kpc->getNitroMinBurst();
    // Magnify the differences
    efficiency = (efficiency - 20.0f) * 1.25f;
    skill_pts += setSkillValues(SKILL_NITRO_EFFICIENCY, efficiency,
                   "nitro.png", "nitro", _("Nitro efficiency"));

    // The base time for the skidding bonuses are 1, 2.7 and 4
    // The lower, the better. We add 4 times level 1, 2 times level 2 and 1 time level 3
    // to obtain an aggregate value.
    // We proceed similarly for the skid bonuses (4.0, 6.0, 8.0 by default)
    float aggregate_skid_time = 4 * kpc->getSkidTimeTillBonus()[0] +
                                2 * kpc->getSkidTimeTillBonus()[1] +
                                    kpc->getSkidTimeTillBonus()[2];
    // The lower the kart speed, the easier it is to fit long boosts in various curves,
    // and the less a shorter boost time gives an advantage.
    float missing_speed_frac = (kpc->getEngineGenericMaxSpeed() - kpc->getEngineMaxSpeed())
                              / kpc->getEngineGenericMaxSpeed();
    aggregate_skid_time = aggregate_skid_time * (1.0f - missing_speed_frac)
                         + 13.4f * missing_speed_frac; // 13.4 is the generic value 
    // Make this parameter more significant
    aggregate_skid_time *= sqrtf(aggregate_skid_time);

    float aggregate_bonus_speed = 4 * kpc->getSkidBonusSpeed()[0] +
                                  2 * kpc->getSkidBonusSpeed()[1] +
                                      kpc->getSkidBonusSpeed()[2];

    // Lower turn radiuses make it easier to start and keep drifts going
    float aggregate_turn_radius = kpc->getTurnRadius().get(kpc->getEngineMaxSpeed() - 5.0f) +
                                  kpc->getTurnRadius().get(kpc->getEngineMaxSpeed())        +
                                  kpc->getTurnRadius().get(kpc->getEngineMaxSpeed() + 5.0f);

    float drift_composite = -30.0f + (1100.0f * aggregate_bonus_speed)
                                     / (aggregate_skid_time * sqrtf(aggregate_turn_radius));

    skill_pts += setSkillValues(SKILL_SKIDDING, drift_composite,
                   "android/drift.png", "skidding", _("Drifting bonuses"));

    //Log::verbose("Stats Widget", "The sum of displayed skill values for the selected kart is %f", skill_pts);
    
    RaceManager::get()->setDifficulty(previous_difficulty);
}   // setValues

// -----------------------------------------------------------------------------
float KartStatsWidget::setSkillValues(Stats skill_type, float value, const std::string icon_name,
                                     const std::string skillbar_propID, const irr::core::stringw icon_tooltip)
{
    // Beautify the display by limiting the possibility of very close values
    // that don't quite match in different skill bars.
    value = 1.5f * std::round(value * 0.6667f);
    m_skills[skill_type]->setValue(value);
    m_skills[skill_type]->setIcon(irr::core::stringc(
            file_manager->getAsset(FileManager::GUI_ICON, icon_name).c_str()));    
    m_skills[skill_type]->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_"+skillbar_propID, m_player_id);
    m_skills[skill_type]->m_iconbutton->setTooltip(icon_tooltip);
    return value;
}   // setSkillValues

// -----------------------------------------------------------------------------
void KartStatsWidget::hideAll()
{
    for (SkillLevelWidget* skills : m_skills)
        skills->setVisible(false);
}   // hideAll

// -----------------------------------------------------------------------------
void KartStatsWidget::add()
{
    for (int i = 0; i < SKILL_COUNT; ++i)
    {
        m_skills[i]->add();
    }
}   // add

// -----------------------------------------------------------------------------
void KartStatsWidget::move(int x, int y, int w, int h)
{
    Widget::move(x,y,w,h);
    setSize(m_x, m_y, m_w, m_h);
    int margin = m_h / SKILL_COUNT * 0.6f;
    int offset = m_h / 5;
    for (int i = 0; i < SKILL_COUNT; ++i)
    {
        m_skills[i]->move(m_skill_bar_x,
                          m_y + offset + margin * i,
                          m_skill_bar_w,
                          m_skill_bar_h);
    }
}   // move

// -----------------------------------------------------------------------------
/** Set value for given type
 */
void KartStatsWidget::setValue(Stats type, float value)
{
    m_skills[type]->setValue(value);
}   // setValue

// -----------------------------------------------------------------------------
/** Get value for given type
 */
float KartStatsWidget::getValue(Stats type)
{
    return m_skills[type]->getValue();
}   // getValue

// -----------------------------------------------------------------------------
/** Set size for widgets inside KartStatsWidget
 */
void KartStatsWidget::setSize(const int x, const int y, const int w, const int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    // -- sizes
    m_skill_bar_w = w - m_w / 16; // make sure the bars can't be out of screen 
    m_skill_bar_h = m_h / SKILL_COUNT / 4;

    m_skill_bar_x = x;
}   // setSize

// -----------------------------------------------------------------------------
void KartStatsWidget::setDisplayIcons(bool display_icons)
{
    for (int i = 0; i < SKILL_COUNT; ++i)
    {
        m_skills[i]->setDisplayIcon(display_icons);
    }
}   // setDisplayIcons
