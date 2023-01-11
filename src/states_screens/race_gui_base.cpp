//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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


#include "states_screens/race_gui_base.hpp"

#include "audio/music_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/referee.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/abstract_kart_animation.hpp"
#include "karts/controller/controller.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/rescue_animation.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/network_config.hpp"
#include "states_screens/race_gui_multitouch.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <GlyphLayout.h>
#include <IrrlichtDevice.h>
#include <ICameraSceneNode.h>

namespace irr
{
    namespace video
    {
        extern bool useCoreContext;
    }
}

RaceGUIBase::RaceGUIBase()
{
    m_enabled_network_spectator = false;
    initSize();
    m_ignore_unimportant_messages = false;
    //I18N: as in "ready, set, go", shown at the beginning of the race
    m_string_ready          = _("Ready!");
    //I18N: as in "ready, set, go", shown at the beginning of the race
    m_string_set            = _("Set!");
    //I18N: as in "ready, set, go", shown at the beginning of the race
    m_string_go             = _("Go!");
    //I18N: Shown when a goal is scored
    m_string_goal           = _("GOAL!");
    // I18N: Shown waiting for other players in network to finish loading or
    // waiting
    m_string_waiting_for_others = _("Waiting for others");
    // I18N: Shown waiting for the server in network if live join or spectate
    m_string_waiting_for_the_server = _("Waiting for the server");

    m_music_icon = irr_driver->getTexture("notes.png");
    if (!m_music_icon)
    {
        Log::error("RaceGuiBase", "Can't find 'notes.png' texture, aborting.");
    }

    m_plunger_face = irr_driver->getTexture("plungerface.png");
    if (!m_plunger_face)
    {
        Log::error("RaceGuiBase",
                   "Can't find 'plungerface.png' texture, aborting.");
    }

    //read frame picture for icons in the mini map.
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
    {   // show the kart direction in soccer
        m_icons_frame = irr_driver->getTexture("icons-frame_arrow.png");
    }
    else
    {
        m_icons_frame = irr_driver->getTexture("icons-frame.png");
    }
    m_icons_kart_list = irr_driver->getTexture("icons-frame.png");
    if (!m_icons_frame)
    {
        Log::error("RaceGuiBase",
                   "Can't find 'icons-frame.png' texture, aborting.");
    }
    if (!m_icons_kart_list)
    {
        Log::error("RaceGuiBase",
                   "Can't find 'icons-frame.png' texture, aborting.");
    }

    m_gauge_full            = irr_driver->getTexture(FileManager::GUI_ICON, "gauge_full.png");
    m_gauge_full_bright     = irr_driver->getTexture(FileManager::GUI_ICON, "gauge_full_bright.png");
    m_gauge_empty           = irr_driver->getTexture(FileManager::GUI_ICON, "gauge_empty.png");
    m_gauge_goal            = irr_driver->getTexture(FileManager::GUI_ICON, "gauge_goal.png");
    m_lap_flag              = irr_driver->getTexture(FileManager::GUI_ICON, "lap_flag.png");
    m_dist_show_overlap     = 2;
    m_icons_inertia         = 2;

    m_referee               = NULL;
    m_multitouch_gui        = NULL;
    m_showing_kart_colors   = false;
}   // RaceGUIBase

// ----------------------------------------------------------------------------
/** Called when loading the race gui or screen resized. */
void RaceGUIBase::initSize()
{
    m_max_font_height       = GUIEngine::getFontHeight() + 10;
    m_small_font_max_height = GUIEngine::getSmallFontHeight() + 5;
}   // initSize

// ----------------------------------------------------------------------------
/** This is a second initialisation call (after the constructor) for the race
 *  gui. This is called after the world has been initialised, e.g. all karts
 *  do exist (while the constructor is called before the karts are created).
 *  In the base gui this is used to initialise the referee data (which needs
 *  the start positions of the karts). Note that this function is (and must
 *  be called only once, after its constructor).
 *  \pre All karts must be created, since this object defines the
 *       positions for the referees based on the karts.
 */
void RaceGUIBase::init()
{
    m_kart_display_infos.resize(RaceManager::get()->getNumberOfKarts());

    // Do everything else required at a race restart as well, esp.
    // resetting the height of the referee.
    m_referee = new Referee();
    m_referee_pos.resize(RaceManager::get()->getNumberOfKarts());
    m_referee_rotation.resize(RaceManager::get()->getNumberOfKarts());
}   // init

//-----------------------------------------------------------------------------
/** This is called when restarting a race. In the race gui base it resets
 *  height of the referee, so that it can start flying down again.
 */
void RaceGUIBase::reset()
{
    // While we actually only need the positions for local players,
    // we add all karts, since it's easier to get a world kart id from
    // the kart then the local player id (and it avoids problems in
    // profile mode where there might be a camera, but no player).
    for(unsigned int i=0; i<RaceManager::get()->getNumberOfKarts(); i++)
    {
        const AbstractKart *kart = World::getWorld()->getKart(i);
        m_referee_pos[i] = kart->getTrans()(Referee::getStartOffset());
        Vec3 hpr;
        btQuaternion q = btQuaternion(kart->getTrans().getBasis().getColumn(1),
            Referee::getStartRotation().getY() * DEGREE_TO_RAD) *
            kart->getTrans().getRotation();
        hpr.setHPR(q);
        m_referee_rotation[i] = hpr.toIrrHPR();
    }

    m_referee_height = 10.0f;
    m_referee->attachToSceneNode();
    m_referee->selectReadySetGo(0); // set red color
    m_plunger_move_time = 0;
    m_plunger_offset    = core::vector2di(0,0);
    m_plunger_speed     = core::vector2df(0,0);
    m_plunger_state     = PLUNGER_STATE_INIT;
    m_showing_kart_colors = false;
    m_enabled_network_spectator = false;
    clearAllMessages();
    
    if (m_multitouch_gui != NULL)
    {
        m_multitouch_gui->reset();
    }
}   // reset

//-----------------------------------------------------------------------------
/** The destructor removes the marker texture and the referee scene node.
 */
RaceGUIBase::~RaceGUIBase()
{
    //irr_driver->removeTexture(m_marker);

    // If the referee is currently being shown,
    // remove it from the scene graph.
    delete m_referee;
}   // ~RaceGUIBase

//-----------------------------------------------------------------------------
void RaceGUIBase::recreateGUI()
{
    if (m_multitouch_gui)
        m_multitouch_gui->recreate();

    initSize();
    calculateMinimapSize();
    
    Track* track = Track::getCurrentTrack();
    assert(track != NULL);
    track->updateMiniMapScale();
}  // recreateGUI

//-----------------------------------------------------------------------------
/** Creates the 2D vertices for a regular polygon. Adopted from Irrlicht.
 *  \param n Number of vertices to use.
 *  \param radius Radius of the polygon.
 *  \param center The center point of the polygon.
 *  \param v Pointer to the array of vertices.
 */
void RaceGUIBase::createRegularPolygon(unsigned int n, float radius,
                                   const core::vector2df &center,
                                   const video::SColor &color,
                                   video::S3DVertex *v,
                                   unsigned short int *index)
{
    float f = 2*M_PI/(float)n;
    for (unsigned int i=0; i<n; i++)
    {
        float p = i*f;
        core::vector2df X = center + core::vector2df( sinf(p)*radius,
                                                     -cosf(p)*radius);
        v[i].Pos.X = X.X;
        v[i].Pos.Y = X.Y;
        v[i].Color = color;
        index[i]   = i;
    }
}   // createRegularPolygon

//-----------------------------------------------------------------------------
/** Displays all messages in the message queue
 **/
void RaceGUIBase::drawAllMessages(const AbstractKart* kart,
                                  const core::recti &viewport,
                                  const core::vector2df &scaling)
{
    int y = viewport.LowerRightCorner.Y - m_small_font_max_height - 10;
    
    const int x = viewport.getCenter().X;
    const int w = viewport.getWidth();

    // Draw less important messages first, at the very bottom of the screen
    // unimportant messages are skipped in multiplayer, they take too much screen space
    if (RaceManager::get()->getNumLocalPlayers() < 2 &&
        !m_ignore_unimportant_messages)
    {
        for (AllMessageType::const_iterator i = m_messages.begin();
             i != m_messages.end(); ++i)
        {
            TimedMessage const &msg = *i;
            if (!msg.m_important)
            {
                // Display only messages for all karts, or messages for this kart
                if (msg.m_kart && msg.m_kart!=kart) continue;

                core::rect<s32> pos(x - w/2, y, x + w/2, y + m_max_font_height);

                gui::ScalableFont* font = GUIEngine::getSmallFont();

                if (msg.m_outline)
                    font->setBlackBorder(true);

                font->draw(
                    core::stringw(msg.m_message.c_str()).c_str(),
                    pos, msg.m_color, true /* hcenter */, true /* vcenter */);

                if (msg.m_outline)
                    font->setBlackBorder(false);

                y -= m_small_font_max_height;
            }
        }
    }

    // First line of text somewhat under the top of the viewport.
    y = viewport.UpperLeftCorner.Y +
        (viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y)/4;

    gui::ScalableFont* font = GUIEngine::getFont();
    gui::ScalableFont* big_font = GUIEngine::getTitleFont();

    int font_height = m_max_font_height;
    if (RaceManager::get()->getNumLocalPlayers() > 2)
    {
        font = GUIEngine::getSmallFont();
    }

    irr_driver->getVideoDriver()->enableMaterial2D(); // seems like we need to remind irrlicht from time to time to use the Material2D

    // The message are displayed in reverse order, so that a multi-line
    // message (addMessage("1", ...); addMessage("2",...) is displayed
    // in the right order: "1" on top of "2"
    for (AllMessageType::const_iterator i = m_messages.begin();
         i != m_messages.end(); ++i)
    {
        TimedMessage const &msg = *i;

        // less important messages were already displayed
        if (!msg.m_important) continue;

        // Display only messages for all karts, or messages for this kart
        if (msg.m_kart && msg.m_kart!=kart) continue;

        core::rect<s32> pos(x - w/2, y, x + w/2, y + font_height);

        if (msg.m_big_font)
        {
            big_font->draw(core::stringw(msg.m_message.c_str()).c_str(),
                           pos, msg.m_color, true /* hcenter */,
                           true /* vcenter */);
            y += GUIEngine::getTitleFontHeight();
        }
        else
        {
            if (msg.m_outline)
                font->setBlackBorder(true);

            font->draw(core::stringw(msg.m_message.c_str()).c_str(),
                       pos, msg.m_color, true /* hcenter */,
                       true /* vcenter */);

            if (msg.m_outline)
                font->setBlackBorder(false);

            y += font_height;
        }
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Removes messages which have been displayed long enough. This function
 *  must be called after drawAllMessages, otherwise messages which are only
 *  displayed once will not be drawn!
 */
void RaceGUIBase::cleanupMessages(const float dt)
{
    AllMessageType::iterator p =m_messages.begin();
    while(p!=m_messages.end())
    {
        if((*p).done(dt))
        {
            p = m_messages.erase(p);
        }
        else
        {
            ++p;
        }
    }
}   // cleanupMessages

//-----------------------------------------------------------------------------
/** Draws the powerup icons on the screen (called once for each player).
 *  \param kart The kart for which to draw the powerup icons.
 *  \param viewport The viewport into which to draw the icons.
 *  \param scaling The scaling to use when drawing the icons.
 */
void RaceGUIBase::drawPowerupIcons(const AbstractKart* kart,
                                   const core::recti &viewport,
                                   const core::vector2df &scaling)
{
#ifndef SERVER_ONLY
    if (UserConfigParams::m_powerup_display == 2) return;

    // If player doesn't have any powerups or has completed race, do nothing.
    const Powerup* powerup = kart->getPowerup();
    if (powerup->getType() == PowerupManager::POWERUP_NOTHING
        || kart->hasFinishedRace()) return;

    int n = kart->getPowerup()->getNum() ;
    int many_powerups = 0;
    if (n<1) return;    // shouldn't happen, but just in case
    if (n>5)
    {
        many_powerups = n;
        n = 1;
    }

    float scale = (float)(std::min(scaling.X, scaling.Y));

    int nSize = (int)(UserConfigParams::m_powerup_size * scale);

    int itemSpacing = (int)(scale * UserConfigParams::m_powerup_size / 2);

    int x1, y1;

    // When there is not much height or set by user, move items on the side
    if ((UserConfigParams::m_powerup_display == 1) || 
        ((float) viewport.getWidth() / (float) viewport.getHeight() > 2.0f))
    {
        x1 = viewport.UpperLeftCorner.X  + 3*(viewport.getWidth()/4)
           - ((n * itemSpacing)/2);
    }
    else
    {
        x1 = viewport.UpperLeftCorner.X  + (viewport.getWidth()/2)
           - ((n * itemSpacing)/2);
    }

    // When the viewport is smaller in splitscreen, reduce the top margin
    if ((RaceManager::get()->getNumLocalPlayers() == 2    &&
        viewport.getWidth() > viewport.getHeight()) ||
        RaceManager::get()->getNumLocalPlayers() >= 3       )
        y1 = viewport.UpperLeftCorner.Y  + (int)(5 * scaling.Y);
    else
        y1 = viewport.UpperLeftCorner.Y  + (int)(20 * scaling.Y);

    int x2 = 0;

    assert(powerup != NULL);
    assert(powerup->getIcon() != NULL);
    video::ITexture *t=powerup->getIcon()->getTexture();
    assert(t != NULL);
    core::rect<s32> rect(core::position2di(0, 0), t->getSize());

    for ( int i = 0 ; i < n ; i++ )
    {
        x2 = (int)((x1+i*itemSpacing) - (itemSpacing / 2));
        core::rect<s32> pos(x2, y1, x2+nSize, y1+nSize);
        draw2DImage(t, pos, rect, NULL,
                                                  NULL, true);
    }   // for i

    if (many_powerups > 0)
    {
        gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
        core::rect<s32> pos(x2+nSize, y1, x2+nSize+nSize, y1+nSize);
        font->setScale(scale / (float)font->getDimension(L"X").Height * 64.0f);
        font->draw(core::stringw(L"x")+StringUtils::toWString(many_powerups),
            pos, video::SColor(255, 255, 255, 255));
        font->setScale(1.0f);
    }
#endif
}   // drawPowerupIcons

// ----------------------------------------------------------------------------
/** Updates lightning related information.
 */
void RaceGUIBase::renderGlobal(float dt)
{

}   // renderGlobal

// ----------------------------------------------------------------------------
/** Update, called once per frame. This updates the height of the referee
 *  (to create a flying down animation).
 *  \param dt Time step size.
 */
void RaceGUIBase::update(float dt)
{
    // E.g. a race result gui does not have a referee
    if(m_referee)
    {
        World *world = World::getWorld();
        // During GO move the referee up again
        if(world->getPhase()==World::SETUP_PHASE)
        {
            m_referee_height = 10.0f;
            m_referee->selectReadySetGo(0);   // set red color
        }
        else if(world->getPhase()==World::GO_PHASE)
        {
            m_referee_height += dt*5.0f;
            m_referee->selectReadySetGo(2);
        }
        else if (world->getPhase()==World::WAIT_FOR_SERVER_PHASE ||
            (NetworkConfig::get()->isNetworking() &&
            world->getPhase()==World::TRACK_INTRO_PHASE))
        {
        }
        else if ((!NetworkConfig::get()->isNetworking() &&
            world->getPhase()==World::TRACK_INTRO_PHASE) ||
            (NetworkConfig::get()->isNetworking() &&
            world->getPhase()==World::SERVER_READY_PHASE))
        {
            m_referee->selectReadySetGo(0);   // set red color
            m_referee_height -= dt*5.0f;
            if(m_referee_height<0)
               m_referee_height = 0;
        }
        else if(world->isStartPhase())  // must be ready or set now
        {
            m_referee_height = 0;
            m_referee->selectReadySetGo(world->getPhase()==World::SET_PHASE
                                        ? 1 : 0);
        }
        else if(world->getPhase()==World::IN_GAME_MENU_PHASE)
        {
            // Don't do anything, without this the next clause
            // would completely remove thunderbird.
        }
        else if(m_referee->isAttached())   // race phase:
        {
            m_referee->removeFromSceneGraph();
        }
    }   // if referee node

    // Check if switching to spectate mode is need
    auto cl = LobbyProtocol::get<ClientLobby>();
    World* w = World::getWorld();
    if (m_enabled_network_spectator || !cl || !w)
        return;

    if (RaceManager::get()->getNumLocalPlayers() != 1 ||
        !RaceManager::get()->modeHasLaps() ||
        !w->isActiveRacePhase())
        return;

    for (unsigned i = 0; i < w->getNumKarts(); i++)
    {
        AbstractKart* k = w->getKart(i);
        if (!k->getController()->isLocalPlayerController()
            || !k->hasFinishedRace())
            continue;
        // Enable spectate mode after 2 seconds which allow the player to
        // release left / right button if they keep pressing it after the
        // finishing line (1 second here because m_network_finish_check_ticks is
        // already 1 second ahead of time when crossing finished line)
        if (k->getNetworkConfirmedFinishTicks() > 0
            && w->getTicksSinceStart() >
            k->getNetworkConfirmedFinishTicks() + stk_config->time2Ticks(1.0f))
        {
            m_enabled_network_spectator = true;
            cl->setSpectator(true);
            static bool msg_shown = false;
            if (getMultitouchGUI() != NULL)
                recreateGUI();
            else if (!msg_shown)
            {
                msg_shown = true;
                cl->addSpectateHelperMessage();
            }
        }
    }

}   // update

// ----------------------------------------------------------------------------
/** This function is called just before rendering the view for each kart. This
 *  is used here to display the referee during the ready-set-go phase.
 *  \param kart The kart whose view is rendered next.
 */
void RaceGUIBase::preRenderCallback(const Camera *camera)
{
    if(m_referee && camera->getKart())
    {
        unsigned int world_id = camera->getKart()->getWorldKartId();
        Vec3 xyz = m_referee_pos[world_id] +
            camera->getKart()->getNormal() * m_referee_height;
        m_referee->setPosition(xyz);
        m_referee->setRotation(m_referee_rotation[world_id]);
    }
}   // preRenderCallback

// ----------------------------------------------------------------------------
void RaceGUIBase::renderPlayerView(const Camera *camera, float dt)
{
    const core::recti &viewport = camera->getViewport();
    const core::vector2df scaling = camera->getScaling();
    const AbstractKart* kart = camera->getKart();
    if(!kart) return;
    
    if (m_multitouch_gui != NULL && !GUIEngine::ModalDialog::isADialogActive())
    {
        m_multitouch_gui->draw(kart, viewport, scaling);
    }
}   // renderPlayerView


//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
 *  certain amount of time (unless time<0, then the message is displayed
 *  once).
 */
void RaceGUIBase::addMessage(const core::stringw &msg,
                             const AbstractKart *kart,
                             float time, const video::SColor &color,
                             bool important, bool big_font, bool outline)
{
    m_messages.push_back(TimedMessage(msg, kart, time, color, important, big_font, outline));
}   // addMessage

//-----------------------------------------------------------------------------
/** Displays the description given for the music currently being played.
 *  This is usually the title and composer.
 */
void RaceGUIBase::drawGlobalMusicDescription()
{
#ifndef SERVER_ONLY
     // show no music description when it's off
    if (!UserConfigParams::m_music) return;

    gui::IGUIFont*       font = GUIEngine::getFont();
    
    const int fheight = font->getDimension(L"X").Height;
    
    float race_time =
        stk_config->ticks2Time(World::getWorld()->getMusicDescriptionTicks());

    // ---- Manage pulsing effect
    float timeProgression = (float)(race_time) /
                            (float)(stk_config->m_music_credit_time);

    const int x_pulse = (int)(sinf(race_time*9.0f)*fheight/4);
    const int y_pulse = (int)(cosf(race_time*9.0f)*fheight/4);

    float resize = 1.0f;
    if (timeProgression < 0.1)
    {
        resize = timeProgression/0.1f;
    }
    else if (timeProgression > 0.9)
    {
        resize = 1.0f - (timeProgression - 0.9f)/0.1f;
    }

    const float resize3 = resize*resize*resize;

    // Get song name, and calculate its size, allowing us to position stuff
    const MusicInformation* mi = music_manager->getCurrentMusic();
    if (!mi) return;

    // I18N: string used to show the song title (e.g. "Sunny Song")
    core::stringw thetext = _("\"%s\"", mi->getTitle());

    core::dimension2d< u32 > textSize = font->getDimension(thetext.c_str());
    int textWidth = textSize.Width;

    int textWidth2 = 0;
    core::stringw thetext_composer;
    if (mi->getComposer()!="")
    {
        // I18N: string used to show the author of the music. (e.g. "Sunny Song" by "John Doe")
        thetext_composer = _("by");
        thetext_composer += " ";
        thetext_composer += mi->getComposer().c_str();
        textWidth2 = font->getDimension(thetext_composer.c_str()).Width;
    }
    const int max_text_size = (int)(irr_driver->getActualScreenSize().Width*2.0f/3.0f);
    if (textWidth  > max_text_size) textWidth  = max_text_size;
    if (textWidth2 > max_text_size) textWidth2 = max_text_size;

    const int ICON_SIZE = fheight*2;
    const int y         = irr_driver->getActualScreenSize().Height - fheight*5/2;
    // the fheight/2 is an arbitrary space left between the note icon and the text
    const int noteX     = (irr_driver->getActualScreenSize().Width / 2)
                        - std::max(textWidth, textWidth2)/2 - ICON_SIZE/2 - fheight;
    const int noteY     = y;
    // the fheight is an arbitrary space left between the note icon and the text
    const int textXFrom = (irr_driver->getActualScreenSize().Width / 2)
                        - std::max(textWidth, textWidth2)/2 + fheight;
    const int textXTo   = (irr_driver->getActualScreenSize().Width / 2)
                        + std::max(textWidth, textWidth2)/2 + fheight;

    // ---- Draw "by" text
    const int text_y = (int)(irr_driver->getActualScreenSize().Height
                       - fheight*2*(resize3) + fheight*(1-resize));

    static const video::SColor white = video::SColor(255, 255, 255, 255);
    if(mi->getComposer()!="")
    {
        core::rect<s32> pos_by(textXFrom, text_y+fheight,
                               textXTo,   text_y+fheight);
        std::vector<gui::GlyphLayout> gls;
        font->initGlyphLayouts(thetext_composer, gls);
        gui::removeHighlightedURL(gls);
        font->draw(gls, pos_by, white, true, true);
    }

    // ---- Draw "song name" text
    core::rect<s32> pos(textXFrom, text_y,
                        textXTo,   text_y);

    std::vector<gui::GlyphLayout> gls;
    font->initGlyphLayouts(thetext, gls);
    gui::removeHighlightedURL(gls);
    font->draw(gls, pos, white, true /* hcenter */,
               true /* vcenter */);

    // Draw music icon
    if (m_music_icon != NULL)
    {
        int iconSizeX = (int)(ICON_SIZE*resize + x_pulse*resize*resize);
        int iconSizeY = (int)(ICON_SIZE*resize + y_pulse*resize*resize);
    
        core::rect<s32> dest(noteX-iconSizeX/2+fheight,
                             noteY-iconSizeY/2+ICON_SIZE/2,
                             noteX+iconSizeX/2+fheight,
                             noteY+iconSizeY/2+ICON_SIZE/2);
        const core::rect<s32> source(core::position2d<s32>(0,0),
                                     m_music_icon->getSize());
    
        draw2DImage(m_music_icon, dest, source, NULL, NULL, true);
    }
#endif
}   // drawGlobalMusicDescription

//-----------------------------------------------------------------------------
void RaceGUIBase::drawGlobalGoal()
{
    static video::SColor color = video::SColor(255, 255, 255, 255);
    core::rect<s32> pos(irr_driver->getActualScreenSize().Width/2,
                        irr_driver->getActualScreenSize().Height/2,
                        irr_driver->getActualScreenSize().Width/2,
                        irr_driver->getActualScreenSize().Height/2);
    gui::IGUIFont* font = GUIEngine::getTitleFont();
    font->draw(m_string_goal.c_str(), pos, color, true, true);
}
// ----------------------------------------------------------------------------
/** Draws the ready-set-go message on the screen.
 */
void RaceGUIBase::drawGlobalReadySetGo()
{
    // This function is called only in a relevant phase,
    // So we can put common elements here

    static video::SColor color = video::SColor(255, 255, 255, 255);
    gui::IGUIFont* font = GUIEngine::getTitleFont();
    int x = irr_driver->getActualScreenSize().Width/2;
    int y = irr_driver->getActualScreenSize().Height*2/5;
    core::rect<s32> pos(x,y,x,y);

    switch (World::getWorld()->getPhase())
    {
    case WorldStatus::WAIT_FOR_SERVER_PHASE:
        {
            font->draw(StringUtils::loadingDots(
                World::getWorld()->isLiveJoinWorld() ?
                m_string_waiting_for_the_server.c_str() :
                m_string_waiting_for_others.c_str()
                ), pos, color, true, true);
        }
        break;
    case WorldStatus::READY_PHASE:
        {
            font->draw(m_string_ready.c_str(), pos, color, true, true);
        }
        break;
    case WorldStatus::SET_PHASE:
        {
            font->draw(m_string_set.c_str(), pos, color, true, true);
        }
        break;
    case WorldStatus::GO_PHASE:
        {
            if (RaceManager::get()->getCoinTarget() > 0)
                font->draw(_("Collect nitro!"), pos, color, true, true);
            else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER)
                font->draw(_("Follow the leader!"), pos, color, true, true);
            else
                font->draw(m_string_go.c_str(), pos, color, true, true);
        }
        break;
    default:
         break;
    }   // switch
}   // drawGlobalReadySetGo

//-----------------------------------------------------------------------------
/** Draw players icons and, depending on the current mode, their time
 *  or their score (battle lives, egg collected, etc.).
 */
void RaceGUIBase::drawGlobalPlayerIcons(int bottom_margin)
{
#ifndef SERVER_ONLY
    const RaceManager::MinorRaceModeType  minor_mode = RaceManager::get()->getMinorMode();
    if (!UserConfigParams::m_soccer_player_list &&
        minor_mode == RaceManager::MINOR_MODE_SOCCER)
        return;

    int x_base = 10;
    if (irr_driver->getDevice()->getLeftPadding() > 0)
        x_base += irr_driver->getDevice()->getLeftPadding();
    int y_base = 25;
    unsigned int y_space = irr_driver->getActualScreenSize().Height - bottom_margin - y_base;
    // Special case : when 3 players play, use 4th window to display such stuff
    if (RaceManager::get()->getIfEmptyScreenSpaceExists())
    {
        irr::core::recti Last_Space = irr_driver->getSplitscreenWindow(RaceManager::get()->getNumLocalPlayers());
        x_base = Last_Space.UpperLeftCorner.X;
        y_base = Last_Space.UpperLeftCorner.Y;
        y_space = irr_driver->getActualScreenSize().Height - y_base;
    }

    unsigned int sta = RaceManager::get()->getNumSpareTireKarts();
    unsigned int total_karts = RaceManager::get()->getNumberOfKarts() - sta;
    unsigned int num_karts = 0;
    if (NetworkConfig::get()->isNetworking())
        num_karts = World::getWorld()->getCurrentNumKarts();
    else
        num_karts = RaceManager::get()->getNumberOfKarts() - sta;
    // May happen in spectate mode if all players disconnected before server
    // reset
    if (num_karts == 0)
        return;

    // -2 because that's the spacing further on
    int ICON_PLAYER_WIDTH = y_space / num_karts - 2;

    int icon_width_max = (int)(60*(irr_driver->getActualScreenSize().Width/1024.0f));
    int icon_width_min = GUIEngine::getFontHeight();
    if (icon_width_min < 35) icon_width_min = 35;
    if (icon_width_min > icon_width_max)
    {
        int icon_width_tmp = icon_width_max;
        icon_width_max = icon_width_min;
        icon_width_min = icon_width_tmp;
    }

    // Make sure it fits within our boundaries
    if (ICON_PLAYER_WIDTH > icon_width_max) ICON_PLAYER_WIDTH = icon_width_max;
    if (ICON_PLAYER_WIDTH < icon_width_min) ICON_PLAYER_WIDTH = icon_width_min;

    // Icon width for the AI karts
    int ICON_WIDTH = ICON_PLAYER_WIDTH * 5 / 6;

    WorldWithRank* world = dynamic_cast<WorldWithRank*>(World::getWorld());

    //initialize m_previous_icons_position
    if(m_previous_icons_position.size()==0)
    {
        for(unsigned int i=0; i<total_karts; i++)
        {
            const AbstractKart *kart = world->getKart(i);
            int position = kart->getPosition();
            core::vector2d<s32> pos(x_base,y_base+(position-1)*(ICON_PLAYER_WIDTH+2));
            m_previous_icons_position.push_back(pos);
        }
    }

    int x;
    int y;
    float previous_distance=0.0;//no need to be far ahead, first kart won't try to overlap



    int previous_x=x_base;
    int previous_y=y_base-ICON_PLAYER_WIDTH-2;

    gui::ScalableFont* font = GUIEngine::getFont();
    const unsigned int kart_amount = world->getNumKarts() - sta;

    //where is the limit to hide last icons
    int y_icons_limit = irr_driver->getActualScreenSize().Height - 
                                            bottom_margin - ICON_PLAYER_WIDTH;
    if (RaceManager::get()->getIfEmptyScreenSpaceExists())
    {
        y_icons_limit = irr_driver->getActualScreenSize().Height - ICON_WIDTH;
    }

    world->getKartsDisplayInfo(&m_kart_display_infos);

    for(int position = 1; position <= (int)kart_amount ; position++)
    {
        AbstractKart *kart = world->getKartAtDrawingPosition(position);

        if (kart->getPosition() == -1)//if position is not set
        {
            //we use karts ordered by id only
            //(needed for beginning of battle mode)
            kart= world->getKart(position-1);
        }

        if (kart->isEliminated() || !kart->isVisible()) continue;
        unsigned int kart_id = kart->getWorldKartId();

        KartIconDisplayInfo &info = m_kart_display_infos[kart_id];
        //x,y is the target position
        int lap = info.lap;

        // In battle mode mode there is no distance along track etc.
        if (minor_mode==RaceManager::MINOR_MODE_3_STRIKES ||
            minor_mode==RaceManager::MINOR_MODE_FREE_FOR_ALL ||
            minor_mode==RaceManager::MINOR_MODE_CAPTURE_THE_FLAG ||
            minor_mode==RaceManager::MINOR_MODE_EASTER_EGG ||
            minor_mode==RaceManager::MINOR_MODE_SOCCER)
        {
            x = x_base;
            y = previous_y+ICON_PLAYER_WIDTH+2;
        }
        else
        {
            LinearWorld *linear_world = (LinearWorld*)(World::getWorld());

            float distance = linear_world->getDistanceDownTrackForKart(kart_id, true)
                           + Track::getCurrentTrack()->getTrackLength()*lap;

            if ((position>1) &&
                (previous_distance-distance<m_dist_show_overlap) &&
                (!kart->hasFinishedRace()) && lap >= 0 )
            {
                //linear translation : form (0,ICON_PLAYER_WIDTH+2) to
                // (previous_x-x_base+(ICON_PLAYER_WIDTH+2)/2,0)
                x=(int)(x_base+(1-(previous_distance-distance)
                                /m_dist_show_overlap)
                        *(previous_x-x_base+(ICON_PLAYER_WIDTH+2)/2));
                y=(int)(previous_y+(previous_distance-distance)
                        /m_dist_show_overlap*(ICON_PLAYER_WIDTH+2));
            }
            else
            {
                x=x_base;
                y=previous_y+ICON_PLAYER_WIDTH+2;
            }
            previous_distance=distance;
        }   // not three-strike-battle


        previous_x=x;//save coord of the previous kart in list
        previous_y=y;

        //soft movement using previous position:
        x=(int)((x+m_previous_icons_position[kart_id].X*m_icons_inertia)
                /(m_icons_inertia+1));
        y=(int)((y+m_previous_icons_position[kart_id].Y*m_icons_inertia)
                /(m_icons_inertia+1));

        //save position for next time
        m_previous_icons_position[kart_id].X=x;
        m_previous_icons_position[kart_id].Y=y;

        if (y>y_icons_limit)
        {
            //there are too many icons, write "Top 9", to express that
            //there is not everybody shown
            core::recti pos_top;
            pos_top.UpperLeftCorner.Y  = y_base-22;
            pos_top.UpperLeftCorner.X  = x_base;

            static video::SColor color = video::SColor(255, 255, 255, 255);
            //I18N: When some GlobalPlayerIcons are hidden, write "Top 10" to show it
            core::stringw top_text = _("Top %i", position - 1);
            core::dimension2du dim = font->getDimension(top_text.c_str());
            pos_top.LowerRightCorner   = pos_top.UpperLeftCorner;
            pos_top.LowerRightCorner.X += dim.Width;
            pos_top.LowerRightCorner.Y += dim.Height;
            font->setBlackBorder(true);
            font->setThinBorder(true);
            font->draw(top_text, pos_top, color);
            font->setThinBorder(false);
            font->setBlackBorder(false);

            break;
        }

        // If player number is large (small icon), rescale font size
        int font_height = font->getDimension(L"X").Height;
        if ((float)ICON_PLAYER_WIDTH*0.7f < (float)font_height)
            font->setScale(0.7f*(float)ICON_PLAYER_WIDTH / (float)font_height);

        if (info.m_text.size() > 0)
        {
            core::dimension2du dim = font->getDimension(info.m_text.c_str());

            core::rect<s32> pos(x + ICON_PLAYER_WIDTH, y + 5,
                                x + ICON_PLAYER_WIDTH + dim.Width, y + 5 + dim.Height);
            if (info.m_outlined_font)
            {

                GUIEngine::getOutlineFont()->draw(info.m_text, pos,
                    info.m_color, false, false, NULL, true/*ignore RTL*/);
            }
            else
            {
                font->setBlackBorder(true);
                font->setThinBorder(true);
                font->draw(info.m_text, pos, info.m_color, false, false, NULL,
                    true/*ignore RTL*/);
                font->setThinBorder(false);
                font->setBlackBorder(false);
            }
        }

        if (info.special_title.size() > 0)
        {
            core::dimension2du dim = font->getDimension(info.special_title.c_str());

            core::rect<s32> pos(x + ICON_PLAYER_WIDTH, y + 5,
                x + ICON_PLAYER_WIDTH + dim.Width, y + 5 + dim.Height);
            core::stringw s(info.special_title.c_str());
            font->setBlackBorder(true);
            font->setThinBorder(true);
            font->draw(s.c_str(), pos, info.m_color, false, false, NULL,
                       true /* ignore RTL */);
            font->setThinBorder(false);
            font->setBlackBorder(false);
        }
        font->setScale(1.0f);


        AbstractKart* target_kart = NULL;
        Camera* cam = Camera::getActiveCamera();
        auto cl = LobbyProtocol::get<ClientLobby>();
        bool is_nw_spectate = cl && cl->isSpectator();
        // For network spectator highlight
        if (RaceManager::get()->getNumLocalPlayers() == 1 && cam && is_nw_spectate)
            target_kart = cam->getKart();
        bool is_local = is_nw_spectate ? kart == target_kart :
            kart->getController()->isLocalPlayerController();

        int w = is_local ? ICON_PLAYER_WIDTH : ICON_WIDTH;
        drawPlayerIcon(kart, x, y, w, is_local);
    } //next position
#endif
}   // drawGlobalPlayerIcons

//-----------------------------------------------------------------------------
/** Draw one player icon
 *  Takes care of icon looking different due to plumber, squashing, ...
 */
void RaceGUIBase::drawPlayerIcon(AbstractKart *kart, int x, int y, int w,
                                 bool is_local)
{
#ifndef SERVER_ONLY
    video::ITexture *icon =
    kart->getKartProperties()->getIconMaterial()->getTexture();

    CaptureTheFlag* ctf = dynamic_cast<CaptureTheFlag*>(World::getWorld());
    unsigned int kart_id = kart->getWorldKartId();

    // CTF
    if (ctf)
    {
        if (ctf->getRedHolder() == (int)kart_id)
        {
            video::ITexture* red =
                irr_driver->getTexture(FileManager::GUI_ICON, "red_flag.png");
            const core::rect<s32> rect(core::position2d<s32>(0, 0),
                red->getSize());
            const core::rect<s32> pos1
                (x - 20, y - 10, x + w - 20, y + w - 30);
            draw2DImage(red, pos1, rect, NULL, NULL, true);
        }
        else if (ctf->getBlueHolder() == (int)kart_id)
        {
            video::ITexture* blue =
                irr_driver->getTexture(FileManager::GUI_ICON, "blue_flag.png");
            const core::rect<s32> rect(core::position2d<s32>(0, 0),
                blue->getSize());
            const core::rect<s32> pos1
                (x - 20, y - 10, x + w - 20, y + w - 30);
            draw2DImage(blue, pos1, rect, NULL, NULL, true);
        }
    }

    const core::rect<s32> pos(x, y, x+w, y+w);

    // Get color of kart
    // Since kart->getKartProperties()->getColor() only gets the
    // standard color of a kart of same type, we have to check if the user
    // (or network manager) changed it. In that case we have to use
    // hue value instead.
    video::SColor kart_color = kart->getKartProperties()->getColor();
    const float kart_hue = RaceManager::get()->getKartColor(kart->getWorldKartId());
    if (kart_hue > 0.0)
    {
        // convert Hue to SColor
        const video::SColorHSL kart_colorHSL(kart_hue * 360.0, 80.0, 50.0);
        video::SColorf kart_colorf;
        kart_colorHSL.toRGB(kart_colorf);
        kart_color = kart_colorf.toSColor();
    }

    //to bring to light the player's icon: add a background
    const RaceManager::MinorRaceModeType  minor_mode = RaceManager::get()->getMinorMode();
    if (is_local && m_icons_kart_list != NULL)
    {
        video::SColor colors[4];
        for (unsigned int i=0;i<4;i++)
        {
            colors[i]=kart_color;
            colors[i].setAlpha(
                               120+(int)(120*cosf(M_PI/2*i+World::getWorld()->getTime()*2)));
        }
        core::rect<s32> icon_pos;
        if (m_showing_kart_colors)
        {
            // we are showing other kart's colors, so draw bigger circle
            icon_pos = core::rect<s32>(x-9, y-7, x+w+7, y+w+2);
        }
        else
        {
            icon_pos = pos;
        }
        const core::rect<s32> rect(core::position2d<s32>(0,0),
                                   m_icons_kart_list->getSize());
        draw2DImage(m_icons_kart_list, icon_pos, rect,NULL, colors, true);
    }
    else if (kart_hue > 0.0 && (minor_mode == RaceManager::MINOR_MODE_NORMAL_RACE
            || minor_mode == RaceManager::MINOR_MODE_TIME_TRIAL))
    {
        // in normal mode or time trial draw kart color circles for karts with custom color
        // draw a little bigger in case an addon kart uses the full icon size
        const core::rect<s32> color_pos(x-7, y+2, x+w, y+w+2);
        video::SColor colors[4] = {kart_color, kart_color, kart_color, kart_color};
        colors[0].setAlpha(240);    // higher alpha for left part
        colors[1].setAlpha(240);
        colors[2].setAlpha(125);    // lower alpha for right part
        colors[3].setAlpha(125);
        const core::rect<s32> rect(core::position2d<s32>(0,0), m_icons_frame->getSize());
        draw2DImage(m_icons_frame, color_pos, rect, NULL, colors, true);
        m_showing_kart_colors = true;
    }

    // Fixes crash bug, why are certain icons not showing up?
    if (icon  && !kart->getKartAnimation() && !kart->isSquashed())
    {
        const core::rect<s32> rect(core::position2d<s32>(0,0),
                                   icon->getSize());
        video::SColor translucence((unsigned)-1);
        translucence.setAlpha(128);
        if (kart->isGhostKart())
            draw2DImage(icon, pos, rect, NULL, translucence, true);
        else
            draw2DImage(icon, pos, rect, NULL, NULL, true);
    }

    //draw status info - icon fade out in case of rescue/explode

    if (icon  && dynamic_cast<RescueAnimation*>(kart->getKartAnimation()))
    {
        //icon fades to the left
        float t = kart->getKartAnimation()->getAnimationTimer();
        float t_anim=100*sinf(0.5f*M_PI*t);
        const core::rect<s32> rect1(core::position2d<s32>(0,0),
                                    icon->getSize());
        const core::rect<s32> pos1((int)(x-t_anim), y,
                                   (int)(x+w-t_anim), y+w);
        draw2DImage(icon, pos1, rect1,
                                                  NULL, NULL, true);
    }

    if (icon  && !kart->getKartAnimation() && kart->isSquashed() )
    {
        //syncs icon squash with kart squash
        const core::rect<s32> destRect(core::position2d<s32>(x,y+w/4),
                                       core::position2d<s32>(x+w,y+w*3/4));
        const core::rect<s32> sourceRect(core::position2d<s32>(0,0),
                                         icon->getSize());
        draw2DImage(icon, destRect,
                                                  sourceRect, NULL, NULL,
                                                  true);
    }

    if (icon  &&
        dynamic_cast<ExplosionAnimation*>(kart->getKartAnimation()) )
    {
        //exploses into 4 parts
        float t = kart->getKartAnimation()->getAnimationTimer();
        float t_anim=50.0f*sinf(0.5f*M_PI*t);
        u16 icon_size_x=icon->getSize().Width;
        u16 icon_size_y=icon->getSize().Height;

        const core::rect<s32> rect1(0, 0, icon_size_x/2,icon_size_y/2);
        const core::rect<s32> pos1((int)(x-t_anim), (int)(y-t_anim),
                                   (int)(x+w/2-t_anim),
                                   (int)(y+w/2-t_anim));
        draw2DImage(icon, pos1, rect1,
                                                  NULL, NULL, true);

        const core::rect<s32> rect2(icon_size_x/2,0,
                                    icon_size_x,icon_size_y/2);
        const core::rect<s32> pos2((int)(x+w/2+t_anim),
                                   (int)(y-t_anim),
                                   (int)(x+w+t_anim),
                                   (int)(y+w/2-t_anim));
        draw2DImage(icon, pos2, rect2,
                                                  NULL, NULL, true);

        const core::rect<s32> rect3(0, icon_size_y/2, icon_size_x/2,icon_size_y);
        const core::rect<s32> pos3((int)(x-t_anim), (int)(y+w/2+t_anim),
                                   (int)(x+w/2-t_anim), (int)(y+w+t_anim));
        draw2DImage(icon, pos3, rect3, NULL, NULL, true);

        const core::rect<s32> rect4(icon_size_x/2,icon_size_y/2,icon_size_x,icon_size_y);
        const core::rect<s32> pos4((int)(x+w/2+t_anim), (int)(y+w/2+t_anim),
                                   (int)(x+w+t_anim), (int)(y+w+t_anim));
        draw2DImage(icon, pos4, rect4, NULL, NULL, true);
    }

    // Current item(s) and how many if > 1
    const Powerup* powerup = kart->getPowerup();
    if (UserConfigParams::m_karts_powerup_gui &&
        powerup->getType() != PowerupManager::POWERUP_NOTHING && !kart->hasFinishedRace())
    {
        int numberItems = kart->getPowerup()->getNum();
        video::ITexture *iconItem = powerup->getIcon()->getTexture();
        assert(iconItem);

        const core::rect<s32> rect(core::position2d<s32>(0,0), iconItem->getSize());
        const core::rect<s32> posItem(x + w/2, y + w/2, x + 5*w/4, y + 5*w/4);
        draw2DImage(iconItem, posItem, rect, NULL, NULL, true);

        if (numberItems > 1)
        {
            gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
            const core::rect<s32> posNumber(x + w, y + w/4, x + 7*w/4, y + w);
            font->setScale(3.f*((float) w)/(4.f*(float)font->getDimension(L"X").Height));
            font->draw(StringUtils::toWString(numberItems), posNumber, video::SColor(255, 255, 255, 255));
            font->setScale(1.0f);
        }
    }

    //Plunger
    if (kart->getBlockedByPlungerTicks()>0)
    {
        video::ITexture *icon_plunger =
        powerup_manager->getIcon(PowerupManager::POWERUP_PLUNGER)->getTexture();
        if (icon_plunger != NULL)
        {
            const core::rect<s32> rect(core::position2d<s32>(0,0),
                                       icon_plunger->getSize());
            const core::rect<s32> pos1(x+10, y-10, x+w+10, y+w-10);
            draw2DImage(icon_plunger, pos1,
                                                      rect, NULL, NULL,
                                                      true);
        }
    }
    //attachment
    if (kart->getAttachment()->getType() != Attachment::ATTACH_NOTHING)
    {
        video::ITexture *icon_attachment =
        attachment_manager->getIcon(kart->getAttachment()->getType())
        ->getTexture();
        if (icon_attachment != NULL)
        {
            const core::rect<s32> rect(core::position2d<s32>(0,0),
                                       icon_attachment->getSize());
            const core::rect<s32> pos1(x-20, y-10, x+w-20, y+w-10);
            draw2DImage(icon_attachment,
                                                      pos1, rect, NULL,
                                                      NULL, true);
        }
    }

    //lap flag for finished karts
    if (kart->hasFinishedRace())
    {
        if (m_lap_flag != NULL)
        {
            const core::rect<s32> rect(core::position2d<s32>(0, 0),
                m_lap_flag->getSize());
            const core::rect<s32> pos1(x - 20, y - 10, x + w - 20, y + w - 10);
            draw2DImage(m_lap_flag, pos1, rect, NULL, NULL, true);
        }
    }
#endif
}   // drawPlayerIcon

// ----------------------------------------------------------------------------

/** Draws the plunger-in-face if necessary. Does nothing if there is no
 *  plunger in face atm.
 */
void RaceGUIBase::drawPlungerInFace(const Camera *camera, float dt)
{
#ifndef SERVER_ONLY
    const AbstractKart *kart = camera->getKart();
    if (kart->getBlockedByPlungerTicks()<=0)
    {
        m_plunger_state = PLUNGER_STATE_INIT;
        return;
    }

    const core::recti &viewport = camera->getViewport();

    const int screen_width = viewport.LowerRightCorner.X
                           - viewport.UpperLeftCorner.X;

    if(m_plunger_state == PLUNGER_STATE_INIT)
    {
        m_plunger_move_time = 0.0f;
        m_plunger_offset    = core::vector2di(0,0);
        m_plunger_state     = PLUNGER_STATE_SLOW_2;
        m_plunger_speed     = core::vector2df(0, 0);
    }

    if(World::getWorld()->getPhase()!=World::IN_GAME_MENU_PHASE)
    {
        m_plunger_move_time -= dt;
        if(m_plunger_move_time < dt && m_plunger_state!=PLUNGER_STATE_FAST)
        {
            const float fast_time = 0.3f;
            if(kart->getBlockedByPlungerTicks()<stk_config->time2Ticks(fast_time))
            {
                // First time we reach faste state: select random target point
                // at top of screen and set speed accordingly
                RandomGenerator random;
                float movement_fraction = 0.3f;
                int plunger_x_target  = screen_width/2
                    + random.get((int)(screen_width*movement_fraction))
                    - (int)(screen_width*movement_fraction*0.5f);
                m_plunger_state = PLUNGER_STATE_FAST;
                m_plunger_speed =
                    core::vector2df((plunger_x_target-screen_width/2)/fast_time,
                    viewport.getHeight()*0.5f/fast_time);
                m_plunger_move_time = fast_time;
            }
            else
            {
                RandomGenerator random;
                m_plunger_move_time = 0.1f+random.get(50)/200.0f;
                // Plunger is either moving or not moving
                if(m_plunger_state==PLUNGER_STATE_SLOW_1)
                {
                    m_plunger_state = PLUNGER_STATE_SLOW_2;
                    m_plunger_speed =
                        core::vector2df(0, 0.05f*viewport.getHeight()
                        /m_plunger_move_time      );
                }
                else
                {
                    m_plunger_state = PLUNGER_STATE_SLOW_1;
                    m_plunger_speed =
                        core::vector2df(0, 0.02f*viewport.getHeight()
                        /m_plunger_move_time      );
                }
            }   // has not reach fast moving state
        }

        m_plunger_offset.X += (int)(m_plunger_speed.X * dt);
        m_plunger_offset.Y += (int)(m_plunger_speed.Y * dt);
    }

    if (m_plunger_face != NULL)
    {
        const int plunger_size = (int)(0.6f * screen_width);
        int offset_y = viewport.UpperLeftCorner.Y + viewport.getHeight()/2
                     - plunger_size/2 - m_plunger_offset.Y;
    
        int plunger_x = viewport.UpperLeftCorner.X + screen_width/2
                      - plunger_size/2;
    
        plunger_x += (int)m_plunger_offset.X;
        core::rect<s32> dest(plunger_x,              offset_y,
                             plunger_x+plunger_size, offset_y+plunger_size);
    
        const core::rect<s32> source(core::position2d<s32>(0,0),
                                     m_plunger_face->getSize());
    
        draw2DImage(m_plunger_face, dest, source,
                                                  &viewport /* clip */,
                                                  NULL /* color */,
                                                  true /* alpha */     );
    }
#endif   // !SERVER_ONLY
}   // drawPlungerInFace

// ----------------------------------------------------------------------------
void RaceGUIBase::removeReferee()
{
    if (m_referee->isAttached())   // race phase:
    {
        m_referee->removeFromSceneGraph();
    }
}   // removeReferee

