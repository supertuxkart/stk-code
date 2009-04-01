
#include "gui/state_manager.hpp"
#include "gui/engine.hpp"
#include "gui/widget.hpp"
#include "gui/screen.hpp"
#include "input/input_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "race_manager.hpp"
#include "network/network_manager.hpp"
#include "main_loop.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart.hpp"

#include <vector>

namespace StateManager
{
    void showTrackSelectionScreen()
    {
        pushMenu("tracks.stkgui");
        GUIEngine::RibbonGridWidget* w = dynamic_cast<GUIEngine::RibbonGridWidget*>(GUIEngine::getCurrentScreen()->getWidget("tracks"));
        assert( w != NULL );
        
        // FIXME - should be called only once, not on every show?
        w->addItem("Track 1","t1","track1.png");
        w->addItem("Track 2","t2","track2.png");
        w->addItem("Track 3","t3","track3.png");
        w->addItem("Track 4","t4","track4.png");
        w->addItem("Track 5","t5","track5.png");
        w->addItem("Track 6","t6","track6.png");
        w->addItem("Track 7","t7","track7.png");
        w->addItem("Track 8","t8","track8.png");
        w->updateItemDisplay();
    }
  
    
    void menuEventMain(GUIEngine::Widget* widget, std::string& name)
    {
        GUIEngine::RibbonWidget* ribbon = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
        if(ribbon == NULL) return; // only interesting stuff in main menu is the ribbons
        std::string selection = ribbon->getSelectionName().c_str();
        
        
        if(selection == "new")
        {
            StateManager::pushMenu("karts.stkgui");
            GUIEngine::RibbonGridWidget* w = dynamic_cast<GUIEngine::RibbonGridWidget*>(GUIEngine::getCurrentScreen()->getWidget("karts"));
            assert( w != NULL );
            
            // FIXME - should be called only once, not on every show?
            // FIXME - move to a menu show-up callback
            w->addItem("Gnu","k1","gnu.png");
            w->addItem("Wilber","k2","gnu.png");
            w->addItem("Tux","k3","gnu.png");
            w->addItem("Puffy","k4","gnu.png");
            w->addItem("Hexley","k5","gnu.png");
            w->addItem("Sushi","k6","gnu.png");
            w->addItem("Nolok","k7","gnu.png");
            w->addItem("Mozilla","k8","gnu.png");
            w->updateItemDisplay();
            
            GUIEngine::SpinnerWidget* w2 = dynamic_cast<GUIEngine::SpinnerWidget*>
            (GUIEngine::getCurrentScreen()->getWidget("player"));
            assert( w2 != NULL );
            w2->addLabel("Hiker");
            w2->addLabel("Auria");
            w2->addLabel("Conso");
            w2->addLabel("MiniBjorn");
            
            GUIEngine::ModelViewWidget* w3 = dynamic_cast<GUIEngine::ModelViewWidget*>
            (GUIEngine::getCurrentScreen()->getWidget("modelview"));
            
            assert( w3 != NULL );
            
            // set kart model
            IMesh* mesh = kart_properties_manager->getKart("tux")->getKartModel()->getModel();
            SAnimatedMesh* test = new SAnimatedMesh(); // FIXME - memory management
            test->addMesh(mesh);
            //test->setMaterialFlag(EMF_LIGHTING , false);
            
            w3->setModel(test);
        }
        else if(selection == "options")
        {
            StateManager::pushMenu("options.stkgui");
        }
        else if(selection == "quit")
        {
            main_loop->abort();
            return;
        }
        else if (selection == "options")
        {
            pushMenu("options_av.stkgui");
        }
    }
    
    void menuEventKarts(GUIEngine::Widget* widget, std::string& name)
    {
        // TODO - actually check which kart was selected
        if(name == "karts")
        {
            StateManager::pushMenu("racesetup.stkgui");
        }
    }
    
    void menuEventRaceSetup(GUIEngine::Widget* widget, std::string& name)
    {
        // TODO - detect difficulty, allow more game modes
        if(name == "gamemode")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
            if(w->getSelectionName() == "normal")
            {
                showTrackSelectionScreen();
            }
        }
        /*
         289         race_manager->setDifficulty((RaceManager::Difficulty)m_difficulty);
         290         user_config->setDefaultNumDifficulty(m_difficulty);
         
         // if there is no AI, there's no point asking the player for the amount of karts.
         299     // It will always be the same as the number of human players
         300     if(RaceManager::isBattleMode( race_manager->getMinorMode() ))
         301     {
         302         race_manager->setNumKarts(race_manager->getNumLocalPlayers());
         303         // Don't change the default number of karts in user_config
         304     }
         305     else
         306     {
         307         race_manager->setNumKarts(m_num_karts);
         308         user_config->setDefaultNumKarts(race_manager->getNumKarts());
         309     }
         
         311     if( race_manager->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX    &&
         312         RaceManager::modeHasLaps( race_manager->getMinorMode() )    )
         313     {
         314         race_manager->setNumLaps( m_num_laps );
         315         user_config->setDefaultNumLaps(m_num_laps);
         316     }
         317     // Might still be set from a previous challenge
         318     race_manager->setCoinTarget(0);
         
         input_manager->setMode(InputManager::INGAME);
         
         race_manager->setLocalKartInfo(0, argv[i+1]);
         
         race_manager->setDifficulty(RaceManager::RD_EASY);
         race_manager->setDifficulty(RaceManager::RD_HARD);
         race_manager->setDifficulty(RaceManager::RD_HARD);
         
         race_manager->setTrack(argv[i+1]);
         
         user_config->setDefaultNumKarts(stk_config->m_max_karts);
         race_manager->setNumKarts(user_config->getDefaultNumKarts() );
         
         user_config->getDefaultNumKarts()
         
         StateManager::enterGameState();
         race_manager->startNew();
         */
        
    }
    void menuEventTracks(GUIEngine::Widget* widget, std::string& name)
    {
        // -- track seelction screen
        if(name == "tracks")
        {
            GUIEngine::RibbonGridWidget* w2 = dynamic_cast<GUIEngine::RibbonGridWidget*>(widget);
            if(w2 != NULL)
            {
                std::cout << "Clicked on track " << w2->getSelectionName().c_str() << std::endl;
                
                StateManager::enterGameState();
                race_manager->setLocalKartInfo(0, "tux");
                race_manager->setDifficulty(RaceManager::RD_HARD);
                race_manager->setTrack("beach");
                race_manager->setNumLaps( 3 );
                race_manager->setCoinTarget( 0 ); // Might still be set from a previous challenge
                race_manager->setNumKarts( 1 );
                race_manager->setNumPlayers( 1 );
                race_manager->setNumLocalPlayers( 1 );
                network_manager->setupPlayerKartInfo();
                //race_manager->getKartType(1) = KT_PLAYER;
                
                race_manager->startNew();
            }
        }
        else if(name == "gps")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
            if(w != NULL)
                std::cout << "Clicked on GrandPrix " << w->getSelectionName().c_str() << std::endl;
        }    
        
    }
    void menuEventOptions(GUIEngine::Widget* widget, std::string& name)
    {
        // -- options
        if(name == "options_choice")
        {
            std::string selection = ((GUIEngine::RibbonWidget*)widget)->getSelectionName().c_str();
            
            if(selection == "audio_video") replaceTopMostMenu("options_av.stkgui");
            else if(selection == "players") replaceTopMostMenu("options_players.stkgui");
            else if(selection == "controls") replaceTopMostMenu("options_input.stkgui");
            
            // select the right tab - FIXME - add some kind of post-screen-activation callback to do this
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(GUIEngine::getCurrentScreen()->getWidget("options_choice"));
            if(w != NULL)
            {
                w->select(selection);
            }
            
        }
    }
    
    
    void eventCallback(GUIEngine::Widget* widget, std::string& name)
    {
        std::cout << "event!! " << name.c_str() << std::endl;
        
        const std::string& screen_name = GUIEngine::getCurrentScreen()->getName();
        
        if( screen_name == "main.stkgui" )
            menuEventMain(widget, name);
        else if( screen_name == "karts.stkgui" )
            menuEventKarts(widget, name);
        else if( screen_name == "racesetup.stkgui" )
            menuEventRaceSetup(widget, name);
        else if( screen_name == "tracks.stkgui" )
            menuEventTracks(widget, name);
        else if( screen_name == "options_av.stkgui" || screen_name == "options_input.stkgui" || screen_name == "options_players.stkgui")
            menuEventOptions(widget, name);
        else
            std::cerr << "Warning, unknown menu " << screen_name << " in event callback\n";
        

        
        
    }
    
    void initGUI()
    {
        IrrlichtDevice* device = irr_driver->getDevice();
        video::IVideoDriver* driver = device->getVideoDriver();
        GUIEngine::init(device, driver, &eventCallback);
    }
    
    std::vector<std::string> g_menu_stack;
    static bool g_game_mode = false;
    
    void pushMenu(std::string name)
    {
        input_manager->setMode(InputManager::MENU);
        g_menu_stack.push_back(name);
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
    }
    void replaceTopMostMenu(std::string name)
    {
        input_manager->setMode(InputManager::MENU);
        g_menu_stack[g_menu_stack.size()-1] = name;
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
    }
    
    void popMenu()
    {
        g_menu_stack.pop_back();
        
        if(g_menu_stack.size() == 0)
        {
            main_loop->abort();
            return;
        }
        
        g_game_mode = g_menu_stack[g_menu_stack.size()-1] == "race";
        GUIEngine::switchToScreen(g_menu_stack[g_menu_stack.size()-1].c_str()); 
    }
    
    void resetAndGoToMenu(std::string name)
    {
        race_manager->exitRace();
        input_manager->setMode(InputManager::MENU);
        g_menu_stack.clear();
        g_menu_stack.push_back(name);
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
    }
    
    void enterGameState()
    {
        g_menu_stack.clear();
        g_menu_stack.push_back("race");
        g_game_mode = true;
        GUIEngine::clear();
        input_manager->setMode(InputManager::INGAME);
    }
    
    bool isGameState()
    {
        return g_game_mode;
    }
    
    void escapePressed()
    {
        if(g_game_mode)
        {
            resetAndGoToMenu("main.stkgui");
        }
        else
        {
            popMenu();
        }
    }
    
}
