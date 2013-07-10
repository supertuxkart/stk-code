#include "modes/tutorial_world.hpp"

#include "karts/kart.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"

TutorialWorld::TutorialWorld()
{
    m_stop_music_when_dialog_open = false;
}   // TutorialWorld