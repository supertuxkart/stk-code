//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 SuperTuxKart-Team
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

#include "config/player_profile.hpp"

#include "achievements/achievements_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "online/online_player_profile.hpp"
#include "utils/string_utils.hpp"

//------------------------------------------------------------------------------
/** Constructor to create a new player that didn't exist before.
 *  \param name Name of the player.
 *  \param is_guest True if this is a guest account.
*/
PlayerProfile::PlayerProfile(const core::stringw& name, bool is_guest)
{
#ifdef DEBUG
    m_magic_number = 0xABCD1234;
#endif
    m_local_name          = name;
    m_is_guest_account    = is_guest;
    m_use_frequency       = is_guest ? -1 : 0;
    m_unique_id           = PlayerManager::get()->getUniqueId();
    m_saved_session       = false;
    m_saved_token         = "";
    m_saved_user_id       = 0;
    m_last_online_name    = "";
    m_last_was_online     = false;
    m_remember_password   = false;
    m_default_kart_color  = 0.0f;
    initRemainingData();
}   // PlayerProfile

//------------------------------------------------------------------------------
/** Constructor to deserialize player data that was saved to a XML file. The
 *  constructor will only load the main player data (like name, id, saved
 *  online data), but not the achievements and story mode data. Reason is
 *  that the achievement and story mode data depends on other data to be
 *  read first (challenges and achievement files), which in turn can only be
 *  created later in the startup process (they depend on e.g. all tracks to
 *  be known). On the other hand, automatic login needs to happen asap (i.e.
 *  as soon as the network thread is started) to avoid the player having to
 *  wait for the login to finish , which needs the main player data (i.e.
 *  the default player, and saved session data). So the constructor only
 *  reads this data, the rest of the player data is handled in
 *  loadRemainingData later in the initialisation process.
 *  \param node The XML node representing this player.
*/
PlayerProfile::PlayerProfile(const XMLNode* node)
{
    m_saved_session       = false;
    m_saved_token         = "";
    m_saved_user_id       = 0;
    m_last_online_name    = "";
    m_last_was_online     = false;
    m_remember_password   = false;
    m_story_mode_status   = NULL;
    m_achievements_status = NULL;
    m_default_kart_color  = 0.0f;
    m_icon_filename       = "";

    node->getAndDecode("name",      &m_local_name);
    node->get("guest",              &m_is_guest_account );
    node->get("use-frequency",      &m_use_frequency    );
    node->get("unique-id",          &m_unique_id        );
    node->get("saved-session",      &m_saved_session    );
    node->get("saved-user",         &m_saved_user_id    );
    node->get("saved-token",        &m_saved_token      );
    node->getAndDecode("last-online-name",   &m_last_online_name );
    node->get("last-was-online",    &m_last_was_online  );
    node->get("remember-password",  &m_remember_password);
    node->get("icon-filename",      &m_icon_filename    );
    node->get("default-kart-color", &m_default_kart_color);
    #ifdef DEBUG
    m_magic_number = 0xABCD1234;
    #endif

}   // PlayerProfile

//------------------------------------------------------------------------------
PlayerProfile::~PlayerProfile()
{
    delete m_story_mode_status;
    delete m_achievements_status;
#ifdef DEBUG
    m_magic_number = 0xDEADBEEF;
#endif
}   // ~PlayerProfile


//------------------------------------------------------------------------------
/** This function loads the achievement and story mode data. These can only
 *  be loaded after the UnlockManager is created, which needs the karts
 *  and tracks to be loaded first.
 */
void PlayerProfile::loadRemainingData(const XMLNode *node)
{
    const XMLNode *xml_story_mode = node->getNode("story-mode");
    m_story_mode_status =
                  unlock_manager->createStoryModeStatus(xml_story_mode);
    const XMLNode *xml_achievements = node->getNode("achievements");
    m_achievements_status = AchievementsManager::get()
                          ->createAchievementsStatus(xml_achievements, m_unique_id == 1);
    // Fix up any potentially missing icons.
    addIcon();
}   // initRemainingData

//------------------------------------------------------------------------------
/** Initialises the story- and achievement data structure in case of the first
 *  start of STK.
 */
void PlayerProfile::initRemainingData()
{
    m_story_mode_status = unlock_manager->createStoryModeStatus();
    m_achievements_status =
        AchievementsManager::get()->createAchievementsStatus();
    addIcon();
}   // initRemainingData

//------------------------------------------------------------------------------
/** Creates an icon for a player if non exist so far. It takes the unique
 *  player id modulo the number of karts to pick an icon from the karts. It
 *  then uses the unique number plus the extentsion of the original icon as the
 *  file name (it's not possible to use the player name, since the name is in
 *  utf-16, but typically the file systems are not, resulting in incorrect file
 *  names). The icon is then copied to the user config directory, so that it
 *  can be replaced by an icon made by the user.
 *  If there should be an error copying the file, the icon filename is set
 *  to "". Every time stk is started, it will try to fix missing icons
 *  (which allows it to start from old/incompatible config files).
 *  \pre This function must only be called after all karts are read in.
 */
void PlayerProfile::addIcon()
{
    if (m_icon_filename.size() > 0 || isGuestAccount())
        return;

    int n = (m_unique_id + kart_properties_manager->getKartId("tux") - 1)
          % kart_properties_manager->getNumberOfKarts();

    std::string source = kart_properties_manager->getKartById(n)
                                                ->getAbsoluteIconFile();
    // Create the filename for the icon of this player: the unique id
    // followed by .png or .jpg.
    std::ostringstream out;
    out << m_unique_id <<"."<<StringUtils::getExtension(source);
    if(file_manager->copyFile(source,
                               file_manager->getUserConfigFile(out.str())) )
    {
        m_icon_filename = out.str();
    }
    else
    {
        m_icon_filename = "";
    }
}   // addIcon

//------------------------------------------------------------------------------
/** Returns the name of the icon file for this player. If the player icon
 *  file is undefined, it returns a "?" mark texture. Note, getAsset does
 *  not return a reference, but only a temporary string. So we must return a
 *  copy of the string (not a reference to).
 */
const std::string PlayerProfile::getIconFilename() const
{
    // If the icon file is undefined or does not exist, return the "?" icon
    if(m_icon_filename.size()==0 ||
       !file_manager->fileExists(file_manager->getUserConfigFile(m_icon_filename)))
    {
        return file_manager->getAsset(FileManager::GUI_ICON, "main_help.png");
    }

    return file_manager->getUserConfigFile(m_icon_filename);
}   // getIconFilename

//------------------------------------------------------------------------------
/** Writes the data for this player to the specified UTFWriter.
 *  \param out The utf writer to write the data to.
 */
void PlayerProfile::save(UTFWriter &out)
{
    out << "    <player name=\"" << StringUtils::xmlEncode(m_local_name)
        << "\" guest=\""         << m_is_guest_account
        << "\" use-frequency=\"" << m_use_frequency << "\"\n";

    out << "            icon-filename=\"" << m_icon_filename << "\"\n";

    out << "            unique-id=\""  << m_unique_id
        << "\" saved-session=\""       << m_saved_session << "\"\n";

    out << "            saved-user=\"" << m_saved_user_id
        << "\" saved-token=\""         << m_saved_token << "\"\n";
    out << "            last-online-name=\"" << StringUtils::xmlEncode(m_last_online_name)
        << "\" last-was-online=\""           << m_last_was_online << "\"\n";
    out << "            remember-password=\""         << m_remember_password << "\"\n";
    out << "            default-kart-color=\""        << m_default_kart_color << "\">\n";
    {
        bool is_current_player = false;
        PlayerProfile *player = PlayerManager::getCurrentPlayer();

        if (player != NULL && (getName() == player->getName()))
            is_current_player = true;

        if(m_story_mode_status)
            m_story_mode_status->save(out, is_current_player);

        if(m_achievements_status)
            m_achievements_status->save(out);
    }
    out << "    </player>\n";
}   // save

//------------------------------------------------------------------------------
// ------------------------------------------------------------------------
/** Saves the online data, so that it will automatically re-connect
*  next time this profile is loaded.
*  \param user_id Id of the online profile.
*  \param token Token used for authentication.
*/
void PlayerProfile::saveSession(int user_id, const std::string &token)
{
    m_saved_session = true;
    m_saved_user_id = user_id;
    m_saved_token   = token;
    PlayerManager::get()->save();
}   // saveSession

// ------------------------------------------------------------------------
/** Unsets any saved session data. */
void PlayerProfile::clearSession(bool save)
{
    m_saved_session = false;
    m_saved_user_id = 0;
    m_saved_token   = "";
    if(save)
        PlayerManager::get()->save();
}   // clearSession

//------------------------------------------------------------------------------
/** Increments how often that account was used. Guest accounts are not counted.
 */
void PlayerProfile::incrementUseFrequency()
{
    if (m_is_guest_account) m_use_frequency = -1;
    else m_use_frequency++;
}   // incrementUseFrequency

// ------------------------------------------------------------------------
/** Notification of a finished race, which can trigger fulfilling
*  challenges. */
void PlayerProfile::raceFinished()
{
    m_story_mode_status->raceFinished();
    m_achievements_status->onRaceEnd();
}   // raceFinished

//------------------------------------------------------------------------------
/** Comparison used to sort players.
 */
bool PlayerProfile::operator<(const PlayerProfile &other)
{
    return m_use_frequency < other.m_use_frequency;
}   // operator<

// -----------------------------------------------------------------------------

