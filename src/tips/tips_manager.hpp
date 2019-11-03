//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2019 SuperTuxKart-Team
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

#ifndef HEADER_TIPS_MANAGER_HPP
#define HEADER_TIPS_MANAGER_HPP

#include <string>
#include <map>

class Tip
{
private:
    /** The content of the tip.
    */
    std::string m_content;
    
    /** The icon path of the tip.
    */
    std::string m_icon_path;

public:
    std::string getContent() const { return m_content; }
    // ----------------------------------------------------------------------------------------
    std::string getIconPath() const { return m_icon_path; }
} // Tip

/** This class manages the list of all tips. It reads the
 *  data/tips.xml file, which contains the contents for
 *  each tip.
  */
class TipsManager
{
private:
    /** Pointer to the single instance. */
    static AchievementsManager* m_achievements_manager;

    std::map<uint32_t, AchievementInfo *> m_achievements_info;

    TipsManager      ();
    ~TipsManager     ();
    AchievementsStatus * createNewSlot(unsigned int id, bool online);

public:
    /** Static function to create the instance of the achievement manager. */
    static void create()
    {
        
    }   // create
    // ------------------------------------------------------------------------
    /** Static function to get the achievement manager. */
    static AchievementsManager* get()
    {
        assert(m_achievements_manager);
        return m_achievements_manager;
    }   // get
    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_achievements_manager;
        m_achievements_manager = NULL;
    }   // destroy
    // ========================================================================

    AchievementInfo* getAchievementInfo(uint32_t id) const;
    AchievementsStatus* createAchievementsStatus(const XMLNode *node=NULL);
    // ------------------------------------------------------------------------
    const std::map<uint32_t, AchievementInfo *> & getAllInfo()
    {
        return m_achievements_info;
    }  // getAllInfo

};   // class AchievementsManager

#endif

/*EOF*/
