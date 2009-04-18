
#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
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
#include "user_config.hpp"

#include <vector>

/**
 * This stack will contain menu names (e.g. main.stkgui), and/or 'race'.
 */
std::vector<std::string> g_menu_stack;

static bool g_game_mode = false;

#if 0
#pragma mark Callbacks
#endif

namespace StateManager
{

    /**
     * Callback handling events from the main menu
     */
    void menuEventMain(GUIEngine::Widget* widget, std::string& name)
    {
        GUIEngine::RibbonWidget* ribbon = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
        if(ribbon == NULL) return; // only interesting stuff in main menu is the ribbons
        std::string selection = ribbon->getSelectionName().c_str();
        
        
        if(selection == "new")
        {
            StateManager::pushMenu("karts.stkgui");
        }
        else if(selection == "options")
        {
            StateManager::pushMenu("options_av.stkgui");
        }
        else if(selection == "quit")
        {
            main_loop->abort();
            return;
        }
        else if (selection == "help")
        {
            pushMenu("help1.stkgui");
        }
    }
    
    /**
     * Callback handling events from the kart selection menu
     */
    void menuEventKarts(GUIEngine::Widget* widget, std::string& name)
    {
        static bool karts_menu_inited = false;
        if(name == "init" /*&& !karts_menu_inited*/)
        {
            std::cout << "filling kart menu..." << std::endl;
            GUIEngine::RibbonGridWidget* w = dynamic_cast<GUIEngine::RibbonGridWidget*>(GUIEngine::getCurrentScreen()->getWidget("karts"));
            assert( w != NULL );
            
            if(!karts_menu_inited)
            {
                w->addItem("Gnu","k1","gui/gnu.png");
                w->addItem("Wilber","k2","gui/gnu.png");
                w->addItem("Tux","k3","gui/gnu.png");
                w->addItem("Puffy","k4","gui/gnu.png");
                w->addItem("Hexley","k5","gui/gnu.png");
                w->addItem("Sushi","k6","gui/gnu.png");
                w->addItem("Nolok","k7","gui/gnu.png");
                w->addItem("Mozilla","k8","gui/gnu.png");
                karts_menu_inited = true;
            }
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
            
            // set kart model - FIXME - doesn't work very much
            IMesh* mesh = kart_properties_manager->getKart("tux")->getKartModel()->getModel();
            SAnimatedMesh* test = new SAnimatedMesh(); // FIXME - memory management
            test->addMesh(mesh);
            //test->setMaterialFlag(EMF_LIGHTING , false);
            
            w3->setModel(test);      
            
            //karts_menu_inited = true;
            std::cout << "filled kart menu... done" << std::endl;
        }
        // TODO - actually check which kart was selected
        else if(name == "karts")
        {
            StateManager::pushMenu("racesetup.stkgui");
        }
    }
    
    /**
     * Callback handling events from the race setup menu
     */
    void menuEventRaceSetup(GUIEngine::Widget* widget, std::string& name)
    {
        if(name == "init")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>
                                                (GUIEngine::getCurrentScreen()->getWidget("difficulty"));
            assert( w != NULL );
            w->setSelection(user_config->getDefaultDifficulty());
            
            // TODO - if user arrived to this screen by pressing esc from teh enxt, the behaviour below might be incorrect
            // it would be better to restore previously set settings.
            race_manager->setDifficulty( (RaceManager::Difficulty)user_config->getDefaultDifficulty() );
        }
        else if(name == "difficulty")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
            assert(w != NULL);
            const std::string& selection = w->getSelectionName();
            
            if(selection == "novice")
                race_manager->setDifficulty(RaceManager::RD_EASY);
            else if(selection == "intermediate")
                race_manager->setDifficulty(RaceManager::RD_MEDIUM);
            else if(selection == "expert")
                race_manager->setDifficulty(RaceManager::RD_HARD);
        }
        else if(name == "gamemode")
        {
            // TODO - detect more game modes
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
            if(w->getSelectionName() == "normal")
            {
                StateManager::pushMenu("tracks.stkgui");
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
    
    /**
     * Callback handling events from the track menu
     */
    void menuEventTracks(GUIEngine::Widget* widget, std::string& name)
    {
        static bool track_menu_inited = false;
        if(name == "init")
        {
            GUIEngine::RibbonGridWidget* w = dynamic_cast<GUIEngine::RibbonGridWidget*>
                                                (GUIEngine::getCurrentScreen()->getWidget("tracks"));
            assert( w != NULL );
            
            if(!track_menu_inited)
            {
                w->addItem("Track 1","t1","gui/track1.png");
                w->addItem("Track 2","t2","gui/track2.png");
                w->addItem("Track 3","t3","gui/track3.png");
                w->addItem("Track 4","t4","gui/track4.png");
                w->addItem("Track 5","t5","gui/track5.png");
                w->addItem("Track 6","t6","gui/track6.png");
                w->addItem("Track 7","t7","gui/track7.png");
                w->addItem("Track 8","t8","gui/track8.png");
                track_menu_inited = true;
            }
            w->updateItemDisplay();

        }
        // -- track seelction screen
        if(name == "tracks")
        {
            GUIEngine::RibbonGridWidget* w2 = dynamic_cast<GUIEngine::RibbonGridWidget*>(widget);
            if(w2 != NULL)
            {
                std::cout << "Clicked on track " << w2->getSelectionName().c_str() << std::endl;
                
                StateManager::enterGameState();
                race_manager->setLocalKartInfo(0, "tux");
                //race_manager->setDifficulty(RaceManager::RD_HARD);
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
    
    /**
     * Callback handling events from the options menus
     */
    void menuEventOptions(GUIEngine::Widget* widget, std::string& name)
    {
        // -- init
        if(name == "init")
        {
            const std::string& screen_name = GUIEngine::getCurrentScreen()->getName();
            
            GUIEngine::RibbonWidget* ribbon = dynamic_cast<GUIEngine::RibbonWidget*>
                                            (GUIEngine::getCurrentScreen()->getWidget("options_choice"));
            if(ribbon != NULL)
            {
                if(screen_name == "options_av.stkgui") ribbon->select( "audio_video" );
                else if(screen_name == "options_players.stkgui") ribbon->select( "players" );
                else if(screen_name == "options_input.stkgui") ribbon->select( "controls" );
            }
            
            // audio-video sub-screen
            if(screen_name == "options_av.stkgui")
            {
                GUIEngine::GaugeWidget* gauge = dynamic_cast<GUIEngine::GaugeWidget*>
                    (GUIEngine::getCurrentScreen()->getWidget("sfx_volume"));
                assert(gauge != NULL);
                
                gauge->setValue( sfx_manager->getMasterSFXVolume() );
                
                
                gauge = dynamic_cast<GUIEngine::GaugeWidget*>
                    (GUIEngine::getCurrentScreen()->getWidget("music_volume"));
                assert(gauge != NULL);
                gauge->setValue( sound_manager->getMasterMusicVolume() );
                
                
                GUIEngine::CheckBoxWidget* sfx = dynamic_cast<GUIEngine::CheckBoxWidget*>
                    (GUIEngine::getCurrentScreen()->getWidget("sfx_enabled"));
                
                GUIEngine::CheckBoxWidget* music = dynamic_cast<GUIEngine::CheckBoxWidget*>
                    (GUIEngine::getCurrentScreen()->getWidget("music_enabled"));
                
                sfx->setState( user_config->doSFX() );
                music->setState( user_config->doMusic() );
            }
        }
        // -- options
        else if(name == "options_choice")
        {
            std::string selection = ((GUIEngine::RibbonWidget*)widget)->getSelectionName().c_str();
            
            if(selection == "audio_video") replaceTopMostMenu("options_av.stkgui");
            else if(selection == "players") replaceTopMostMenu("options_players.stkgui");
            else if(selection == "controls") replaceTopMostMenu("options_input.stkgui");
        }
        else if(name == "music_volume")
        {
            GUIEngine::GaugeWidget* w = dynamic_cast<GUIEngine::GaugeWidget*>(widget);
            // GUIEngine::getCurrentScreen()->getWidget("music_volume")
            assert(w != NULL);
            
            // TODO - save value to file, load value from file, allow fully disabling music
            sound_manager->setMasterMusicVolume( w->getValue() );
        }
        else if(name == "sfx_volume")
        {
            GUIEngine::GaugeWidget* w = dynamic_cast<GUIEngine::GaugeWidget*>(widget);
            // GUIEngine::getCurrentScreen()->getWidget("sfx_volume")
            assert(w != NULL);
            
            // TODO - save value to file, load value from file, allow fully disabling sfx
            sfx_manager->setMasterSFXVolume( w->getValue() );
        }
        else if(name == "music_enabled")
        {
            GUIEngine::CheckBoxWidget* w = dynamic_cast<GUIEngine::CheckBoxWidget*>(widget);
            
            user_config->setMusic(w->getState() ? UserConfig::UC_ENABLE : UserConfig::UC_DISABLE);
        }
        else if(name == "sfx_enabled")
        {
            GUIEngine::CheckBoxWidget* w = dynamic_cast<GUIEngine::CheckBoxWidget*>(widget);

            user_config->setSFX(w->getState() ? UserConfig::UC_ENABLE : UserConfig::UC_DISABLE);
        }

    }

    /**
     * Callback handling events from the options menus
     */
    void menuEventHelp(GUIEngine::Widget* widget, std::string& name)
    {
        if(name == "init")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>
            (GUIEngine::getCurrentScreen()->getWidget("category"));
            if(w != NULL)
            {
                const std::string& screen_name = GUIEngine::getCurrentScreen()->getName();
                if(screen_name == "help1.stkgui") w->select( "page1" );
                else if(screen_name == "help2.stkgui") w->select( "page2" );
                else if(screen_name == "help3.stkgui") w->select( "page3" );
            }
        }
        // -- options
        else if(name == "category")
        {
            std::string selection = ((GUIEngine::RibbonWidget*)widget)->getSelectionName().c_str();
            
            if(selection == "page1") replaceTopMostMenu("help1.stkgui");
            else if(selection == "page2") replaceTopMostMenu("help2.stkgui");
            else if(selection == "page3") replaceTopMostMenu("help3.stkgui");
        }
        else if(name == "back")
        {
            StateManager::escapePressed();
        }
    }

    
    /**
     * All widget events will be dispatched to this function; arguments are
     * a pointer to the widget from which the event originates, and its internal
     * name. There is one exception : right after showing a new screen, an event with
     * name 'init' and widget set to NULL will be fired, so the screen can be filled
     * with the right values or so.
     */
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
        else if( screen_name == "help1.stkgui" || screen_name == "help2.stkgui" || screen_name == "help3.stkgui")
            menuEventHelp(widget, name);
        else
            std::cerr << "Warning, unknown menu " << screen_name << " in event callback\n";
   
    }

#if 0
#pragma mark -
#pragma mark Other
#endif
  
    void initGUI()
    {
        IrrlichtDevice* device = irr_driver->getDevice();
        video::IVideoDriver* driver = device->getVideoDriver();
        GUIEngine::init(device, driver, &eventCallback);
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

  
#if 0
#pragma mark -
#pragma mark Push/pop menus
#endif
    
    static std::string g_init_event = "init";
    
    void pushMenu(std::string name)
    {
        input_manager->setMode(InputManager::MENU);
        g_menu_stack.push_back(name);
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
        
        eventCallback(NULL, g_init_event);
    }
    void replaceTopMostMenu(std::string name)
    {
        input_manager->setMode(InputManager::MENU);
        g_menu_stack[g_menu_stack.size()-1] = name;
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
        
        eventCallback(NULL, g_init_event);
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
        
        std::cout << "-- switching to screen " << g_menu_stack[g_menu_stack.size()-1].c_str() << std::endl;
        GUIEngine::switchToScreen(g_menu_stack[g_menu_stack.size()-1].c_str()); 
        
        eventCallback(NULL, g_init_event);
    }
    
    void resetAndGoToMenu(std::string name)
    {
        race_manager->exitRace();
        input_manager->setMode(InputManager::MENU);
        g_menu_stack.clear();
        g_menu_stack.push_back(name);
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
        eventCallback(NULL, g_init_event);
    }
    
}
