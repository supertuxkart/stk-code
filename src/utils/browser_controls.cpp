// Browser-only controls live outside the canvas so they remain discoverable
// during races, battles, and fullscreen play. Native builds keep their own UI.
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

#include "guiengine/modaldialog.hpp"
#include "main_loop.hpp"
#include "modes/world.hpp"
#include "states_screens/state_manager.hpp"

namespace
{
    // Remember only a pause introduced by our confirmation. Cancel must not
    // resume a race that was already paused through its native menu.
    World* confirmation_paused_world = nullptr;
}

extern "C" EMSCRIPTEN_KEEPALIVE int browser_can_pause()
{
    return main_loop && !main_loop->isAborted() && World::getWorld() &&
        StateManager::get()->getGameState() != GUIEngine::MENU;
}

extern "C" EMSCRIPTEN_KEEPALIVE void browser_pause_menu()
{
    if (browser_can_pause())
        StateManager::get()->escapePressed();
}

extern "C" EMSCRIPTEN_KEEPALIVE double browser_play_time()
{
    // Read-only diagnostics let browser regression tests measure the real
    // simulation clock even when the native pause menu hides the canvas HUD.
    return World::getWorld() ? World::getWorld()->getTime() : -1.0;
}

extern "C" EMSCRIPTEN_KEEPALIVE void browser_prepare_quit()
{
    confirmation_paused_world = nullptr;
    World* world = World::getWorld();
    if (main_loop && !main_loop->isAborted() && world &&
        StateManager::get()->getGameState() == GUIEngine::GAME &&
        !GUIEngine::ModalDialog::isADialogActive() &&
        world->getPhase() != WorldStatus::IN_GAME_MENU_PHASE)
    {
        // Pause simulation/audio, not the browser event loop. HTML confirmation
        // remains usable, and the race clock cannot advance behind it.
        world->pause(WorldStatus::IN_GAME_MENU_PHASE);
        confirmation_paused_world = world;
    }
}

extern "C" EMSCRIPTEN_KEEPALIVE void browser_cancel_quit()
{
    if (confirmation_paused_world && World::getWorld() == confirmation_paused_world &&
        confirmation_paused_world->getPhase() == WorldStatus::IN_GAME_MENU_PHASE)
    {
        // unpause() also resets held driving inputs, preventing a stuck throttle
        // when focus has moved between the canvas and confirmation buttons.
        confirmation_paused_world->unpause();
    }
    confirmation_paused_world = nullptr;
}

extern "C" EMSCRIPTEN_KEEPALIVE void browser_quit_game()
{
    confirmation_paused_world = nullptr;
    // Request the ordinary next-frame teardown. It dismisses native dialogs and
    // exits an unfinished world before saving and notifying the browser launcher.
    if (main_loop && !main_loop->isAborted())
        main_loop->requestAbort();
}
#endif
