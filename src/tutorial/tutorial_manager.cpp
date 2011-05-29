//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago
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

#include "tutorial/tutorial_manager.hpp"

#include <set>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"

TutorialManager * m_tutorial_manager=0;
//-----------------------------------------------------------------------------

TutorialManager::TutorialManager()
{
    // The global variable 'tutorial_manager' is needed in the tutorials,
    // but it's not set yet - so we define it here (and it gets re-assign
    // in main).
    m_tutorial_manager = this;
        
    // -----------------------------
    std::set<std::string> result;
    file_manager->listFiles(result, "data/tutorials");
    for(std::set<std::string>::iterator i  = result.begin();
                                        i != result.end()  ; i++)
    {
        if (StringUtils::hasSuffix(*i, ".tutorial")) 
            addTutorial(file_manager->getDataFile("tutorials/"+*i));
    }  

    // Hard coded challenges can be added here.
    load();

}   // UnlockManager

/*-----------------------------------------------------------------------------
** Saves the challenge status.
 */
TutorialManager::~TutorialManager()
{
    //save();
    
    // sfx_manager is destroyed before UnlockManager is, so SFX will be already deleted
    // sfx_manager->deleteSFX(m_locked_sound);
}   // ~UnlockManager

//-----------------------------------------------------------------------------
void TutorialManager::addTutorial(Tutorial * m_tutorial)
{
    m_tutorials_list[m_tutorial->getId()]=m_tutorial;
}   // addTutorial

//-----------------------------------------------------------------------------

void TutorialManager::addTutorial(const std::string& filename)
{
    TutorialData* m_tutorial = NULL;
    try
    {
        m_tutorial = new TutorialData(filename);
        m_tutorial->check();
    }
    catch (std::runtime_error& ex)
    {
        std::cerr << "\n/!\\ An error occurred while loading tutorial file '" << filename << "' : "
                  << ex.what() << " : tutorial will be ignored.\n\n"; 
        if (m_tutorial != NULL) delete m_tutorial;
        return;
    }
    addTutorial(m_tutorial);
    
}   // addChallenge

//-----------------------------------------------------------------------------
std::vector<const Tutorial*> TutorialManager::getTutorialsList()
{
    std::vector<const Tutorial*> all_active;
    for(TutorialsList::iterator i = m_tutorials_list.begin(); 
                                    i!=m_tutorials_list.end();  i++)
    {
        if(i->second->isActive()) all_active.push_back(i->second);
    }
    return all_active;
}   // getActiveChallenges

//-----------------------------------------------------------------------------
const Tutorial* TutorialManager::getTutorial(const std::string& id)
{
    if(m_tutorials_list.find(id) == m_tutorials_list.end()) return NULL;
    return m_tutorials_list[id];
}   // getTutorial

/*-----------------------------------------------------------------------------
** This is called from user_config when reading the config file
*/
void TutorialManager::load()
{
    const std::string filename=file_manager->getChallengeFile("tutorials.xml");
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root || root->getName() != "tutorials")
    {
        std::cerr << "Tutorial file '" << filename << "' will be created." 
                  << std::endl;
        //save();
        
        if (root) delete root;
        return;
    }
    
    for(TutorialsList::iterator i = m_tutorials_list.begin(); 
                                   i!= m_tutorials_list.end();  i++)
    {
       // i->second->load(root);
    }
    delete root;
}   // load


//-----------------------------------------------------------------------------
//void TutorialManager::save()
//{
//    std::ofstream tutorial_file;
    //std::string filename = file_manager->getChallengeFile("tutorial.xml");
    //challenge_file.open(filename.c_str());

    //if(!challenge_file.is_open())
    //{
    //    std::cerr << "Failed to open " << filename << " for writing, challenges won't be saved\n";
    //    return;
    //}

    //challenge_file << "<?xml version=\"1.0\"?>\n";
    //challenge_file << "<challenges>\n";
    //
    //for(AllChallengesType::iterator i = m_all_challenges.begin(); 
    //                                i!= m_all_challenges.end();  i++)
    //{
    //    i->second->save(challenge_file);
    //}
    //
    //challenge_file << "</challenges>\n\n";
    //challenge_file.close();
//}   // save


