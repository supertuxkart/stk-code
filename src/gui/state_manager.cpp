
#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "gui/state_manager.hpp"
#include "gui/engine.hpp"
#include "gui/widget.hpp"
#include "gui/screen.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "race_manager.hpp"
#include "network/network_manager.hpp"
#include "main_loop.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart.hpp"
#include "user_config.hpp"

#include <vector>

using namespace GUIEngine;

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

    // -------------------------------------------------------------------------
    /**
     * Callback handling events from the main menu
     */
    void menuEventMain(Widget* widget, std::string& name)
    {
        RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
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

    // -------------------------------------------------------------------------
    /**
     * Callback handling events from the kart selection menu
     */
    void menuEventKarts(Widget* widget, std::string& name)
    {
        if(name == "init")
        {
            RibbonGridWidget* w = getCurrentScreen()->getWidget<RibbonGridWidget>("karts");
            assert( w != NULL );

            if(!getCurrentScreen()->m_inited)
            {
                w->addItem("Gnu","k1","gui/gnu.png");
                w->addItem("Wilber","k2","gui/gnu.png");
                w->addItem("Tux","k3","gui/gnu.png");
                w->addItem("Puffy","k4","gui/gnu.png");
                w->addItem("Hexley","k5","gui/gnu.png");
                w->addItem("Sushi","k6","gui/gnu.png");
                w->addItem("Nolok","k7","gui/gnu.png");
                w->addItem("Mozilla","k8","gui/gnu.png");
                getCurrentScreen()->m_inited = true;
            }
            w->updateItemDisplay();

            SpinnerWidget* w2 = getCurrentScreen()->getWidget<SpinnerWidget>("player");
            assert( w2 != NULL );
            w2->addLabel("Hiker");
            w2->addLabel("Auria");
            w2->addLabel("Conso");
            w2->addLabel("MiniBjorn");

            ModelViewWidget* w3 = getCurrentScreen()->getWidget<ModelViewWidget>("modelview");

            assert( w3 != NULL );

            // set kart model - FIXME - doesn't work very much
            IMesh* mesh = kart_properties_manager->getKart("tux")->getKartModel()->getModel();
            SAnimatedMesh* test = new SAnimatedMesh(); // FIXME - memory management
            test->addMesh(mesh);
            //test->setMaterialFlag(EMF_LIGHTING , false);

            w3->setModel(test);
            
            getCurrentScreen()->m_inited = true;
        } // end if init
        // TODO - actually check which kart was selected
        else if(name == "karts")
        {
            StateManager::pushMenu("racesetup.stkgui");
        }
    }

    // -------------------------------------------------------------------------
    /**
     * Callback handling events from the race setup menu
     */
    void menuEventRaceSetup(Widget* widget, std::string& name)
    {
        if(name == "init")
        {
            RibbonWidget* w = getCurrentScreen()->getWidget<RibbonWidget>("difficulty");
            assert( w != NULL );
            w->setSelection(user_config->getDefaultDifficulty());

            // TODO - if user arrived to this screen by pressing esc from teh enxt, the behaviour below might be incorrect
            // it would be better to restore previously set settings.
            race_manager->setDifficulty( (RaceManager::Difficulty)user_config->getDefaultDifficulty() );
        }
        else if(name == "difficulty")
        {
            RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
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
            RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
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

    // -------------------------------------------------------------------------
    /**
     * Callback handling events from the track menu
     */
    void menuEventTracks(Widget* widget, std::string& name)
    {
        if(name == "init")
        {
            RibbonGridWidget* w = getCurrentScreen()->getWidget<RibbonGridWidget>("tracks");
            assert( w != NULL );

            if(!getCurrentScreen()->m_inited)
            {
                w->addItem("Track 1","t1","gui/track1.png");
                w->addItem("Track 2","t2","gui/track2.png");
                w->addItem("Track 3","t3","gui/track3.png");
                w->addItem("Track 4","t4","gui/track4.png");
                w->addItem("Track 5","t5","gui/track5.png");
                w->addItem("Track 6","t6","gui/track6.png");
                w->addItem("Track 7","t7","gui/track7.png");
                w->addItem("Track 8","t8","gui/track8.png");
                getCurrentScreen()->m_inited = true;
            }
            w->updateItemDisplay();

        }
        // -- track seelction screen
        if(name == "tracks")
        {
            RibbonGridWidget* w2 = dynamic_cast<RibbonGridWidget*>(widget);
            if(w2 != NULL)
            {
                std::cout << "Clicked on track " << w2->getSelectionName().c_str() << std::endl;

                StateManager::enterGameState();
                //race_manager->setLocalKartInfo(0, "tux");
                race_manager->setLocalKartInfo(0, "nolok");
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
            RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
            if(w != NULL)
                std::cout << "Clicked on GrandPrix " << w->getSelectionName().c_str() << std::endl;
        }

    }

    // -------------------------------------------------------------------------
    /**
     * Callback handling events from the options menus
     */
    void menuEventOptions(Widget* widget, std::string& name)
    {
        // -- init
        if(name == "init")
        {
            const std::string& screen_name = getCurrentScreen()->getName();
            
            RibbonWidget* ribbon = getCurrentScreen()->getWidget<RibbonWidget>("options_choice");
            if(ribbon != NULL)
            {
                if(screen_name == "options_av.stkgui") ribbon->select( "audio_video" );
                else if(screen_name == "options_players.stkgui") ribbon->select( "players" );
                else if(screen_name == "options_input.stkgui") ribbon->select( "controls" );
            }

            // ---- init audio-video sub-screen
            if(screen_name == "options_av.stkgui")
            {
                // ---- sfx volume
                SpinnerWidget* gauge = getCurrentScreen()->getWidget<SpinnerWidget>("sfx_volume");
                assert(gauge != NULL);

                gauge->setValue( (int)(sfx_manager->getMasterSFXVolume()*10.0f) );


                gauge = getCurrentScreen()->getWidget<SpinnerWidget>("music_volume");
                assert(gauge != NULL);
                gauge->setValue( (int)(sound_manager->getMasterMusicVolume()*10.f) );

                // ---- music volume
                CheckBoxWidget* sfx = getCurrentScreen()->getWidget<CheckBoxWidget>("sfx_enabled");

                CheckBoxWidget* music = getCurrentScreen()->getWidget<CheckBoxWidget>("music_enabled");

                // ---- audio enables/disables
                sfx->setState( user_config->doSFX() );
                music->setState( user_config->doMusic() );

                // ---- video modes
                {
                    RibbonGridWidget* res = getCurrentScreen()->getWidget<RibbonGridWidget>("resolutions");
                    assert( res != NULL );
                    
                    
                    CheckBoxWidget* full = getCurrentScreen()->getWidget<CheckBoxWidget>("fullscreen");
                    assert( full != NULL );
                    full->setState( user_config->m_fullscreen );

                    // --- get resolution list from irrlicht the first time
                    if(!getCurrentScreen()->m_inited)
                    {
                        const std::vector<VideoMode>& modes = irr_driver->getVideoModes();
                        const int amount = modes.size();
                        for(int n=0; n<amount; n++)
                        {
                            const int w = modes[n].width;
                            const int h = modes[n].height;
                            const float ratio = (float)w / h;

                            char name[32];
                            sprintf( name, "%ix%i", w, h );

#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)

                            if( ABOUT_EQUAL( ratio, (5.0f/4.0f) ) )
                               res->addItem(name,name,"gui/screen54.png");
                            else if( ABOUT_EQUAL( ratio, (4.0f/3.0f) ) )
                                res->addItem(name,name,"gui/screen43.png");
                            else if( ABOUT_EQUAL( ratio, (16.0f/10.0f) ) )
                                res->addItem(name,name,"gui/screen1610.png");
                            else if( ABOUT_EQUAL( ratio, (5.0f/3.0f) ) )
                                res->addItem(name,name,"gui/screen53.png");
                            else if( ABOUT_EQUAL( ratio, (3.0f/2.0f) ) )
                                res->addItem(name,name,"gui/screen32.png");
                            else
                            {
                                 std::cout << "Unknown screen size ratio : " << ratio << std::endl;
                                // FIXME - do something better than showing a random icon
                                 res->addItem(name,name,"gui/screen1610.png");
                            }
#undef ABOUT_EQUAL
                        } // next resolution

                    } // end if not inited

                    res->updateItemDisplay();

                    // ---- select curernt resolution every time
                    const std::vector<VideoMode>& modes = irr_driver->getVideoModes();
                    const int amount = modes.size();
                    for(int n=0; n<amount; n++)
                    {
                        const int w = modes[n].width;
                        const int h = modes[n].height;

                        char name[32];
                        sprintf( name, "%ix%i", w, h );

                        if(w == user_config->m_width && h == user_config->m_height)
                        {
                            std::cout << "************* Detected right resolution!!! " << n << "\n";
                            // that's the current one
                            res->setSelection(n);
                            break;
                        }
                    }  // end for

                }
            }
            // ---- init input options screen
            else if(screen_name == "options_input.stkgui")
            {
                RibbonGridWidget* devices = getCurrentScreen()->getWidget<RibbonGridWidget>("devices");
                assert( devices != NULL );

                if(!getCurrentScreen()->m_inited)
                {
                    devices->addItem("Keyboard","keyboard","gui/keyboard.png");

                    const int gamepad_count = input_manager->getDeviceList()->getGamePadAmount();

                    for(int i=0; i<gamepad_count; i++)
                    {
                        std::string name = input_manager->getDeviceList()->getGamePad(i)->m_name;
                        char internal_name[32];
                        sprintf(internal_name, "gamepad%i", i);
                        devices->addItem(name,internal_name,"gui/gamepad.png");
                    }

                }
                devices->updateItemDisplay();
            }
            
            getCurrentScreen()->m_inited;
        } // end init

        // -- options
        else if(name == "options_choice")
        {
            std::string selection = ((RibbonWidget*)widget)->getSelectionName().c_str();

            if(selection == "audio_video") replaceTopMostMenu("options_av.stkgui");
            else if(selection == "players") replaceTopMostMenu("options_players.stkgui");
            else if(selection == "controls") replaceTopMostMenu("options_input.stkgui");
        }
        else if(name == "music_volume")
        {
            SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
            assert(w != NULL);

            sound_manager->setMasterMusicVolume( w->getValue()/10.0f );
        }
        else if(name == "sfx_volume")
        {
            static SFXBase* sample_sound = NULL;

            SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
            assert(w != NULL);

            if(sample_sound == NULL)
                sample_sound = sfx_manager->newSFX( SFXManager::SOUND_SKID );
            sample_sound->volume(1);

            sfx_manager->setMasterSFXVolume( w->getValue()/10.0f );
            user_config->m_sfx_volume = w->getValue()/10.0f;

            // play a sample sound to show the user what this volume is like
            sample_sound->position ( Vec3(0,0,0) );

            if(sample_sound->getStatus() != SFXManager::SFX_PLAYING)
            {
                sample_sound->play();
            }

        }
        else if(name == "music_enabled")
        {
            CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

            user_config->setMusic(w->getState() ? UserConfig::UC_ENABLE : UserConfig::UC_DISABLE);

            if(w->getState() == false)
                sound_manager->stopMusic();
            else
                sound_manager->startMusic(sound_manager->getCurrentMusic());
        }
        else if(name == "sfx_enabled")
        {
            CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

            user_config->setSFX(w->getState() ? UserConfig::UC_ENABLE : UserConfig::UC_DISABLE);
        }
        else if(name == "apply_resolution")
        {
            using namespace GUIEngine;
            
            user_config->m_prev_width = user_config->m_width;
            user_config->m_prev_height = user_config->m_height;

            RibbonGridWidget* w1 = getCurrentScreen()->getWidget<RibbonGridWidget>("resolutions");
            assert(w1 != NULL);
            
            const std::string& res = w1->getSelectionName();
            
            int w = -1, h = -1;
            if( sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1 )
            {
                std::cerr << "Failed to decode resolution : " << res.c_str() << std::endl;
                return;
            }
            
            CheckBoxWidget* w2 = getCurrentScreen()->getWidget<CheckBoxWidget>("fullscreen");
            assert(w2 != NULL);
            
            user_config->m_width = w;
            user_config->m_height = h;
            user_config->m_fullscreen = w2->getState();
            irr_driver->changeResolution();
        }
    }

    // -----------------------------------------------------------------------------
    /**
     * Callback handling events from the options menus
     */
    void menuEventHelp(Widget* widget, std::string& name)
    {
        if(name == "init")
        {
            RibbonWidget* w = getCurrentScreen()->getWidget<RibbonWidget>("category");
            
            if(w != NULL)
            {
                const std::string& screen_name = getCurrentScreen()->getName();
                if(screen_name == "help1.stkgui") w->select( "page1" );
                else if(screen_name == "help2.stkgui") w->select( "page2" );
                else if(screen_name == "help3.stkgui") w->select( "page3" );
            }
        }
        // -- options
        else if(name == "category")
        {
            std::string selection = ((RibbonWidget*)widget)->getSelectionName().c_str();

            if(selection == "page1") replaceTopMostMenu("help1.stkgui");
            else if(selection == "page2") replaceTopMostMenu("help2.stkgui");
            else if(selection == "page3") replaceTopMostMenu("help3.stkgui");
        }
        else if(name == "back")
        {
            StateManager::escapePressed();
        }
    }


    // -------------------------------------------------------------------------
    /**
     * All widget events will be dispatched to this function; arguments are
     * a pointer to the widget from which the event originates, and its internal
     * name. There is one exception : right after showing a new screen, an event with
     * name 'init' and widget set to NULL will be fired, so the screen can be filled
     * with the right values or so.
     */
    void eventCallback(Widget* widget, std::string& name)
    {
        std::cout << "event!! " << name.c_str() << std::endl;

        const std::string& screen_name = getCurrentScreen()->getName();

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
        init(device, driver, &eventCallback);
    }

    void enterGameState()
    {
        g_menu_stack.clear();
        g_menu_stack.push_back("race");
        g_game_mode = true;
        cleanForGame();
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
        switchToScreen(name.c_str());

        eventCallback(NULL, g_init_event);
    }
    void replaceTopMostMenu(std::string name)
    {
        input_manager->setMode(InputManager::MENU);
        g_menu_stack[g_menu_stack.size()-1] = name;
        g_game_mode = false;
        switchToScreen(name.c_str());

        eventCallback(NULL, g_init_event);
    }

    void reshowTopMostMenu()
    {
        switchToScreen( g_menu_stack[g_menu_stack.size()-1].c_str() );
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
        switchToScreen(g_menu_stack[g_menu_stack.size()-1].c_str());

        eventCallback(NULL, g_init_event);
    }

    void resetAndGoToMenu(std::string name)
    {
        race_manager->exitRace();
        input_manager->setMode(InputManager::MENU);
        g_menu_stack.clear();
        g_menu_stack.push_back(name);
        g_game_mode = false;
        sound_manager->positionListener( Vec3(0,0,0), Vec3(0,1,0) );
        switchToScreen(name.c_str());
        eventCallback(NULL, g_init_event);
    }

}
