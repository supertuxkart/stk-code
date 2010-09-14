//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "states_screens/main_menu_screen.hpp"

#include <iostream>
#include <string>
//#ifdef ADDONS_MANAGER
//#  include <pthread.h>
//#endif

#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "states_screens/challenges.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "io/file_manager.hpp"

// FIXME : remove, temporary test
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#include "addons/network.hpp"

#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( MainMenuScreen );

// ------------------------------------------------------------------------------------------------------
#ifdef ADDONS_MANAGER
MainMenuScreen::MainMenuScreen() : Screen("mainaddons.stkgui")
{
}
#else
MainMenuScreen::MainMenuScreen() : Screen("main.stkgui")
{
}
#endif
#ifdef ADDONS_MANAGER
void MainMenuScreen::changeNewsText(std::string action)
{
    if(action == "news")
    {
        FILE* newsFile = NULL;
        char buffer[1024] = "";
        newsFile = fopen(std::string(file_manager->getConfigDir() + "/news").c_str(), "r+");
        if (newsFile == NULL)
        {
            fprintf(stderr, "Warning: cannot open news files\n");
            return;
        }
        
        std::string info = std::string("");
        while (fgets(buffer, 1024, newsFile) != NULL)
        {
            info += std::string(buffer);
        }

        fclose(newsFile);
        
        // to remove the break line.
        //info.replace(info.size()-1,1, "");
        //std::cout << info << std::endl;
        pthread_mutex_lock(&(this->m_mutex_news_text));
        m_news_text = std::string(info).c_str();
	    pthread_mutex_unlock(&(this->m_mutex_news_text));
    }
    if(action == "offline")
    {
	    pthread_mutex_lock(&(this->m_mutex_news_text));
        m_news_text = "offline";
	    pthread_mutex_unlock(&(this->m_mutex_news_text));
    }
}
#endif
// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::loadedFromFile()
{
}

// ------------------------------------------------------------------------------------------------------
//
void MainMenuScreen::init()
{
    Screen::init();
    // reset in case we're coming back from a race
    StateManager::get()->resetActivePlayers();
    input_manager->getDeviceList()->setAssignMode(NO_ASSIGN);
    input_manager->setMasterPlayerOnly(false);
#ifdef ADDONS_MANAGER
    pthread_mutex_init(&(this->m_mutex_news_text), NULL);
    pthread_create(&m_thread_news_text, NULL, &MainMenuScreen::downloadNews, this);
#endif
}

#ifdef ADDONS_MANAGER
// ------------------------------------------------------------------------------------------------------
void MainMenuScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
    //FIXME:very bad for performance
    LabelWidget* w = this->getWidget<LabelWidget>("info_addons");
	pthread_mutex_lock(&(this->m_mutex_news_text));
	if(m_news_text == "offline")
	{
	    w->setText("Can't access stkaddons server...");
	}
	else
        w->setText(m_news_text.c_str());
	pthread_mutex_unlock(&(this->m_mutex_news_text));
}
#endif
// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return; // only interesting stuff in main menu is the ribbons
    std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    
    
    if (selection == "network")
    {
        FeatureUnlockedCutScene* scene = FeatureUnlockedCutScene::getInstance();
        
        static int i = 1;
        i++;
        
        if (i % 4 == 0)
        {
            // the passed kart will not be modified, that's why I allow myself to use const_cast
            scene->addUnlockedKart( const_cast<KartProperties*>(kart_properties_manager->getKart("tux")),
                                   L"Unlocked");
            StateManager::get()->pushScreen(scene);
        }
        else if (i % 4 == 1)
        {
            std::vector<video::ITexture*> textures;
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("lighthouse")->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("beach")->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("sandtrack")->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("snowmountain")->getScreenshotFile().c_str()));

            scene->addUnlockedPictures(textures, 1.0, 0.75, L"You did it");
            
            /*
            scene->addUnlockedPicture( irr_driver->getTexture(track_manager->getTrack("lighthouse")->getScreenshotFile().c_str()),
                                      1.0, 0.75, L"You did it");
            */
            
            StateManager::get()->pushScreen(scene);
        }
        else if (i % 4 == 2)
        {
            GrandPrixWin* scene = GrandPrixWin::getInstance();
            const std::string winners[] = { "mriceblock", "nolok", "pidgin" };
            StateManager::get()->pushScreen(scene);
            scene->setKarts( winners );
        }
        else
        {
            GrandPrixLose* scene = GrandPrixLose::getInstance();
            StateManager::get()->pushScreen(scene);
            std::vector<std::string> losers;
            losers.push_back("nolok");
            losers.push_back("tux");
            losers.push_back("wilber");
            scene->setKarts( losers );
        }
    }
    
    if (selection == "new")
    {
        StateManager::get()->pushScreen( KartSelectionScreen::getInstance() );
    }
    else if (selection == "options")
    {
        StateManager::get()->pushScreen( OptionsScreenVideo::getInstance() );
    }
    else if (selection == "quit")
    {
        main_loop->abort();
        return;
    }
    else if (selection == "about")
    {
        StateManager::get()->pushScreen(CreditsScreen::getInstance());
    }
    else if (selection == "help")
    {
        StateManager::get()->pushScreen(HelpScreen1::getInstance());
    }
    else if (selection == "challenges")
    {
        StateManager::get()->pushScreen(ChallengesScreen::getInstance());
    }
#ifdef ADDONS_MANAGER
    else if (selection == "addons")
    {
        std::cout << "Addons" << std::endl;
        StateManager::get()->pushScreen(AddonsScreen::getInstance());
    }
#endif
}

#ifdef ADDONS_MANAGER
// ------------------------------------------------------------------------------------------------------
void * MainMenuScreen::downloadNews( void * pthis)
{
    // FIXME: this code is wrong, "pt" might have been deleted by the
    // time the download is done (by having switched to another screen,
    // or exiting the game, etc...)
    MainMenuScreen * pt = (MainMenuScreen*)pthis;

    if(download("news"))
        pt->changeNewsText("news");
    else
        pt->changeNewsText("offline");
    return NULL;
}
#endif
// ------------------------------------------------------------------------------------------------------
#ifdef ADDONS_MANAGER
MainMenuScreen::~MainMenuScreen()
{
    fprintf(stdout, "canceling the thread");
    pthread_cancel(m_thread_news_text);
}
#endif
