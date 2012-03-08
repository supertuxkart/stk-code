//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  define _WINSOCKAPI_
#  ifdef WIN32
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif

#include "audio/music_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/referee.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

#include <ICameraSceneNode.h>

RaceGUIBase::RaceGUIBase()
{
    m_ignore_unimportant_messages = false;
    m_lightning             = 0.0f;
    m_max_font_height       = GUIEngine::getFontHeight() + 10;
    m_small_font_max_height = GUIEngine::getSmallFontHeight() + 5;
    //I18N: as in "ready, set, go", shown at the beginning of the race
    m_string_ready          = _("Ready!");
    m_string_set            = _("Set!");
    m_string_go             = _("Go!");
    // Make the two materials permanent (in case that they are not listed
    // in the textures/materials.xml file).
    m_music_icon            = material_manager->getMaterial("notes.png",
                                                            /*full path*/false,
                                                            /*permanent*/true);
    m_plunger_face          = material_manager->getMaterial("plungerface.png", 
                                                            /*full path*/false,
                                                            /*permanent*/true);
    //read frame picture for icons in the mini map.
    m_icons_frame           = material_manager->getMaterial("icons-frame.png", 
                                                            /*full_path*/false, 
                                                            /*permanent*/true);
    const std::string &guid = file_manager->getGUIDir();
    m_gauge_full            = irr_driver->getTexture(guid+"gauge_full.png" );
    m_gauge_empty           = irr_driver->getTexture(guid+"gauge_empty.png");
    m_gauge_goal            = irr_driver->getTexture(guid+"gauge_goal.png" );

    m_dist_show_overlap = 2;
    m_icons_inertia     = 2;
    
    
    //I18N: When some GlobalPlayerIcons are hidden, write "Top 10" to show it
    m_string_top        = _("Top %i");
    
    m_referee           = NULL;
}   // RaceGUIBase

// ----------------------------------------------------------------------------
/** This is a second initialisation call (after the constructor) for the race
 *  gui. This is called after the world has been initialised, e.g. all karts
 *  do exist (while the constructor is called before the karts are created).
 *  In the base gui this is used to initialise the referee data (which needs
 *  the start positions of the karts).
 */
void RaceGUIBase::init()
{
    // While we actually only need the positions for local players,
    // we add all karts, since it's easier to get a world kart id from
    // the kart then the local player id (and it avoids problems in 
    // profile mode where there might be a camera, but no player).
    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        const Kart *kart = World::getWorld()->getKart(i);
        m_referee_pos.push_back(kart->getTrans()(Referee::getStartOffset()));
        Vec3 hpr = Referee::getStartRotation() 
                 + Vec3(0, kart->getHeading()*RAD_TO_DEGREE, 0);
        m_referee_rotation.push_back(hpr);
    }

    m_referee = new Referee();

    // Do everything else required at a race restart as well, esp. 
    // resetting the height of the referee.
    restartRace();
}   // init

//-----------------------------------------------------------------------------
/** This is called when restarting a race. In the race gui base it resets
 *  height of the referee, so that it can start flying down again.
 */
void RaceGUIBase::restartRace()
{
    m_referee_height = 10.0f;
    m_referee->attachToSceneNode();
}   // restartRace

//-----------------------------------------------------------------------------
/** The destructor removes the marker texture and the referee scene node.
 */
RaceGUIBase::~RaceGUIBase()
{
    irr_driver->removeTexture(m_marker);

    // If the referee is currently being shown, 
    // remove it from the scene graph.
    delete m_referee;
}   // ~RaceGUIBase

//-----------------------------------------------------------------------------
/** Creates a texture with the markers for all karts in the current race
 *  on it. This assumes that nothing is attached to the scene node at
 *  this stage.
 */
void RaceGUIBase::createMarkerTexture()
{
    unsigned int num_karts = race_manager->getNumberOfKarts();
    unsigned int npower2   = 1;
    // Textures must be power of 2, so 
    while(npower2<num_karts) npower2*=2;

    int radius     = (m_marker_rendered_size>>1)-1;
    IrrDriver::RTTProvider rttProvider(core::dimension2du(m_marker_rendered_size
                                                          *npower2,
                                                          m_marker_rendered_size),
                                     "RaceGUI::markers", true);
    scene::ICameraSceneNode *camera = irr_driver->addCameraSceneNode();
    core::matrix4 projection;
    projection.buildProjectionMatrixOrthoLH((float)(m_marker_rendered_size*npower2), 
                                            (float)(m_marker_rendered_size), 
                                            -1.0f, 1.0f);
    camera->setProjectionMatrix(projection, true);
    core::vector3df center( (float)(m_marker_rendered_size*npower2>>1),
                            (float)(m_marker_rendered_size>>1), 0.0f);
    camera->setPosition(center);
    camera->setUpVector(core::vector3df(0,1,0));
    camera->setTarget(center + core::vector3df(0,0,4));
    // The call to render sets the projection matrix etc. So we have to call 
    // this now before doing the direct OpenGL calls.
    // FIXME: perhaps we should use three calls to irr_driver: begin(),
    // render(), end() - so we could do the rendering by calling to
    // draw2DPolygon() between render() and end(), avoiding the
    // call to camera->render()
    camera->render();
    // We have to reset the material here, since otherwise the last
    // set material (i.e from the kart selection screen) will be used
    // when rednering to the texture.
    video::SMaterial m;
    m.setTexture(0, NULL);
    irr_driver->getVideoDriver()->setMaterial(m);
    for(unsigned int i=0; i<num_karts; i++)
    {
        const std::string& kart_ident = race_manager->getKartIdent(i);
        assert(kart_ident.size() > 0);
        
        const KartProperties *kp=kart_properties_manager->getKart(kart_ident);
        assert(kp != NULL);
        
        core::vector2df center((float)((m_marker_rendered_size>>1)
                                +i*m_marker_rendered_size), 
                               (float)(m_marker_rendered_size>>1)  );
        int count = kp->getShape();
        video::ITexture *t = kp->getMinimapIcon();
        if(t)
        {
            video::ITexture *t = kp->getIconMaterial()->getTexture();
            core::recti dest_rect(i*m_marker_rendered_size, 
                                  0,
                                  (i+1)*m_marker_rendered_size,
                                  m_marker_rendered_size);
            core::recti source_rect(core::vector2di(0,0), t->getSize());
            irr_driver->getVideoDriver()->draw2DImage(t, dest_rect, 
                                                      source_rect,
                                                      /*clipRect*/0,
                                                      /*color*/   0,
                                                      /*useAlpha*/true);
        }
        else   // no special minimap icon defined
        {
            video::S3DVertex *vertices = new video::S3DVertex[count+1];
            unsigned short int *index  = new unsigned short int[count+1];
            video::SColor color        = kp->getColor();
            createRegularPolygon(count, (float)radius, center, color, 
                vertices, index);
            irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(vertices, 
                                            count, index, count-2,
                                            video::EVT_STANDARD, 
                                            scene::EPT_TRIANGLE_FAN);
            delete [] vertices;
            delete [] index;
        }   // if special minimap icon defined
    }

    m_marker = rttProvider.renderToTexture(-1, /*is_2d_render*/true);
    irr_driver->removeCameraSceneNode(camera);
}   // createMarkerTexture

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
        core::vector2df X = center + core::vector2df( sin(p)*radius, 
                                                     -cos(p)*radius);
        v[i].Pos.X = X.X;
        v[i].Pos.Y = X.Y;
        v[i].Color = color;
        index[i]   = i;
    }
}   // createRegularPolygon

//-----------------------------------------------------------------------------
/** Displays all messages in the message queue
 **/
void RaceGUIBase::drawAllMessages(const Kart* kart,
                                  const core::recti &viewport, 
                                  const core::vector2df &scaling)
{    
    int y = viewport.LowerRightCorner.Y - m_small_font_max_height - 10;
          
    const int x = (viewport.LowerRightCorner.X + viewport.UpperLeftCorner.X)/2;
    const int w = (viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X);
    
    // Draw less important messages first, at the very bottom of the screen
    // unimportant messages are skipped in multiplayer, they take too much screen space
    if (race_manager->getNumLocalPlayers() < 2 && 
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
                GUIEngine::getSmallFont()->draw(
                    core::stringw(msg.m_message.c_str()).c_str(),
                    pos, msg.m_color, true /* hcenter */, true /* vcenter */);
                y -= m_small_font_max_height;                    
            }
        }
    }
    
    // First line of text somewhat under the top of the viewport.
    y = (int)(viewport.UpperLeftCorner.Y + 164*scaling.Y);

    gui::ScalableFont* font = GUIEngine::getFont();
    gui::ScalableFont* big_font = GUIEngine::getTitleFont();

    int font_height = m_max_font_height;
    if (race_manager->getNumLocalPlayers() > 2)
    {
        font = GUIEngine::getSmallFont();
        font_height = m_small_font_max_height;
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
            font->draw(core::stringw(msg.m_message.c_str()).c_str(),
                       pos, msg.m_color, true /* hcenter */, 
                       true /* vcenter */);
            y += font_height;
        }
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Removes messages which have been displayed long enough. This function
 *  must be called after drawAllMessages, otherwise messages which are only
 *  displayed once will not be drawn!
 **/
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
 *  \param scaling The scaling to use when draing the icons.
 */
void RaceGUIBase::drawPowerupIcons(const Kart* kart, 
                                   const core::recti &viewport, 
                                   const core::vector2df &scaling)
{
    // If player doesn't have any powerups or has completed race, do nothing.
    const Powerup* powerup = kart->getPowerup();
    if (powerup->getType() == PowerupManager::POWERUP_NOTHING
        || kart->hasFinishedRace()) return;

    int n = kart->getNumPowerup() ;
    if (n<1) return;    // shouldn't happen, but just in case
    if (n>5) n=5;       // Display at most 5 items

    int nSize = (int)(64.0f*std::min(scaling.X, scaling.Y));
        
    int itemSpacing = (int)(std::min(scaling.X, scaling.Y)*30);
    
    int x1 = viewport.UpperLeftCorner.X  + viewport.getWidth()/2 
           - (n * itemSpacing)/2;
    int y1 = viewport.UpperLeftCorner.Y  + (int)(20 * scaling.Y);

    assert(powerup != NULL);
    assert(powerup->getIcon() != NULL);
    video::ITexture *t=powerup->getIcon()->getTexture();
    assert(t != NULL);
    core::rect<s32> rect(core::position2di(0, 0), t->getOriginalSize());
    
    for ( int i = 0 ; i < n ; i++ )
    {
        int x2 = (int)(x1+i*itemSpacing);
        core::rect<s32> pos(x2, y1, x2+nSize, y1+nSize);
        irr_driver->getVideoDriver()->draw2DImage(t, pos, rect, NULL, 
                                                  NULL, true);
    }   // for i
}   // drawPowerupIcons

// ----------------------------------------------------------------------------
/** Updates lightning related information.
*/
void RaceGUIBase::renderGlobal(float dt)
{
    if (m_lightning > 0.0f) m_lightning -= dt;

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
        else if(world->getPhase()==World::TRACK_INTRO_PHASE)
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
}   // update

// ----------------------------------------------------------------------------
/** This function is called just before rendering the view for each kart. This
 *  is used here to display the referee during the ready-set-go phase.
 *  \param kart The kart whose view is rendered next.
 */
void RaceGUIBase::preRenderCallback(const Kart &kart)
{
    if(m_referee)
    {
        Vec3 xyz = m_referee_pos[kart.getWorldKartId()];
        xyz.setY(xyz.getY()+m_referee_height);
        m_referee->setPosition(xyz);
        m_referee->setRotation(m_referee_rotation[kart.getWorldKartId()]);
    }
}   // preRenderCallback

// ----------------------------------------------------------------------------
void RaceGUIBase::renderPlayerView(const Kart *kart)
{
    const core::recti &viewport    = kart->getCamera()->getViewport();

    if (m_lightning > 0.0f)
    {
        GLint glviewport[4];
        glviewport[0] = viewport.UpperLeftCorner.X;
        glviewport[1] = viewport.UpperLeftCorner.Y;
        glviewport[2] = viewport.LowerRightCorner.X;
        glviewport[3] = viewport.LowerRightCorner.Y;
        //glGetIntegerv(GL_VIEWPORT, glviewport);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glColor4f(0.7f*m_lightning, 0.7f*m_lightning, 0.7f*std::min(1.0f, m_lightning*1.5f), 1.0f);
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_CULL_FACE);
        glBegin(GL_QUADS);
        
        glVertex3d(glviewport[0],glviewport[1],0);
        glVertex3d(glviewport[0],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[1],0);
        glEnd();
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
#if 0 // Rainy look, off, TODO: needs to be settable per track
    else
    {
        GLint glviewport[4];
        glGetIntegerv(GL_VIEWPORT, glviewport);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_CULL_FACE);
        glBegin(GL_QUADS);
        
        glVertex3d(glviewport[0],glviewport[1],0);
        glVertex3d(glviewport[0],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[1],0);
        glEnd();
        glEnable(GL_BLEND);
    }
#endif
}   // renderPlayerView


//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
 *  certain amount of time (unless time<0, then the message is displayed
 *  once).
 **/
void RaceGUIBase::addMessage(const core::stringw &msg, const Kart *kart, 
                             float time, const video::SColor &color,
                             bool important, bool big_font)
{
    m_messages.push_back(TimedMessage(msg, kart, time, color, important, big_font));
}   // addMessage

//-----------------------------------------------------------------------------
// Displays the description given for the music currently being played -
// usually the title and composer.
void RaceGUIBase::drawGlobalMusicDescription()
{
     // show no music description when it's off
    if (!UserConfigParams::m_music) return;
    
    gui::IGUIFont*       font = GUIEngine::getFont();

    float race_time = World::getWorld()->getTime();
    // In follow the leader the clock counts backwards, so convert the
    // countdown time to time since start:
    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        race_time = ((FollowTheLeaderRace*)World::getWorld())->getClockStartTime()
                  - race_time;
    // ---- Manage pulsing effect
    // 3.0 is the duration of ready/set (TODO: don't hardcode)
    float timeProgression = (float)(race_time) /
                            (float)(stk_config->m_music_credit_time - 2.0f);
    
    const int x_pulse = (int)(sin(race_time*9.0f)*10.0f);
    const int y_pulse = (int)(cos(race_time*9.0f)*10.0f);
    
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
    
    core::stringw thetext = core::stringw(L"\"") + mi->getTitle() + L"\"";
    
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
    const int max_text_size = (int)(UserConfigParams::m_width*2.0f/3.0f);
    if (textWidth  > max_text_size) textWidth  = max_text_size;
    if (textWidth2 > max_text_size) textWidth2 = max_text_size;

    const int ICON_SIZE = 64;
    const int y         = UserConfigParams::m_height - 80;
    // the 20 is an arbitrary space left between the note icon and the text
    const int noteX     = (UserConfigParams::m_width / 2) 
                        - std::max(textWidth, textWidth2)/2 - ICON_SIZE/2 - 20;
    const int noteY     = y;
    // the 20 is an arbitrary space left between the note icon and the text
    const int textXFrom = (UserConfigParams::m_width / 2) 
                        - std::max(textWidth, textWidth2)/2 + 20;
    const int textXTo   = (UserConfigParams::m_width / 2) 
                        + std::max(textWidth, textWidth2)/2 + 20;

    // ---- Draw "by" text
    const int text_y = (int)(UserConfigParams::m_height - 80*(resize3) 
                     + 40*(1-resize));
    
    static const video::SColor white = video::SColor(255, 255, 255, 255);
    if(mi->getComposer()!="")
    {
        core::rect<s32> pos_by(textXFrom, text_y+40,
                               textXTo,   text_y+40);
        font->draw(thetext_composer, pos_by, white, 
                   true, true);
    }
    
    // ---- Draw "song name" text
    core::rect<s32> pos(textXFrom, text_y,
                        textXTo,   text_y);
    
    font->draw(thetext.c_str(), pos, white, true /* hcenter */, 
               true /* vcenter */);
 
    // Draw music icon
    int iconSizeX = (int)(ICON_SIZE*resize + x_pulse*resize*resize);
    int iconSizeY = (int)(ICON_SIZE*resize + y_pulse*resize*resize);
    
    video::ITexture *t = m_music_icon->getTexture();
    core::rect<s32> dest(noteX-iconSizeX/2+20,    
                         noteY-iconSizeY/2+ICON_SIZE/2,
                         noteX+iconSizeX/2+20,
                         noteY+iconSizeY/2+ICON_SIZE/2);
    const core::rect<s32> source(core::position2d<s32>(0,0), 
                                 t->getOriginalSize());
    
    irr_driver->getVideoDriver()->draw2DImage(t, dest, source,
                                              NULL, NULL, true);
}   // drawGlobalMusicDescription

// ----------------------------------------------------------------------------
/** Draws the ready-set-go message on the screen.
 */
void RaceGUIBase::drawGlobalReadySetGo()
{
    switch (World::getWorld()->getPhase())
    {
    case WorldStatus::READY_PHASE:
        {
            static video::SColor color = video::SColor(255, 255, 255, 255);
            core::rect<s32> pos(UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1,
                                UserConfigParams::m_height>>1);
            gui::IGUIFont* font = GUIEngine::getTitleFont();
            font->draw(m_string_ready.c_str(), pos, color, true, true);
        }
        break;
    case WorldStatus::SET_PHASE:
        {
            static video::SColor color = video::SColor(255, 255, 255, 255);
            core::rect<s32> pos(UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1);
            gui::IGUIFont* font = GUIEngine::getTitleFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font->draw(m_string_set.c_str(), pos, color, true, true);
        }
        break;
    case WorldStatus::GO_PHASE:
        {
            static video::SColor color = video::SColor(255, 255, 255, 255);
            core::rect<s32> pos(UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1);
            //gui::IGUIFont* font = irr_driver->getRaceFont();
            gui::IGUIFont* font = GUIEngine::getTitleFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font->draw(m_string_go.c_str(), pos, color, true, true);
        }
        break;
    default: 
         break;
    }   // switch
}   // drawGlobalReadySetGo

//-----------------------------------------------------------------------------
// Draw players icons and their times (if defined in the current mode).
void RaceGUIBase::drawGlobalPlayerIcons(const KartIconDisplayInfo* info,
                                        int bottom_margin)
{
    int x_base = 10;
    int y_base = 20;
    int ICON_WIDTH=(int)(40*(UserConfigParams::m_width/800.0f));
    int ICON_PLAYER_WIDTH=(int)(50*(UserConfigParams::m_width/800.0f));
    if(UserConfigParams::m_height<600)
    {
        ICON_WIDTH        = 27;
        ICON_PLAYER_WIDTH = 35;
    }
    
    // Special case : when 3 players play, use 4th window to display such stuff
    if (race_manager->getNumLocalPlayers() == 3)
    {
        x_base = UserConfigParams::m_width/2 + x_base;
        y_base = UserConfigParams::m_height/2 + y_base;
    }
    
    WorldWithRank *world    = (WorldWithRank*)(World::getWorld());
    //initialize m_previous_icons_position
    if(m_previous_icons_position.size()==0)
    {
        for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
        {
            const Kart *kart = world->getKart(i);
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
    const unsigned int kart_amount = world->getNumKarts();
    
    //where is the limit to hide last icons
    int y_icons_limit=UserConfigParams::m_height-bottom_margin-ICON_PLAYER_WIDTH;
    if (race_manager->getNumLocalPlayers() == 3)
        y_icons_limit=UserConfigParams::m_height-ICON_WIDTH;
    
    
    for(int position = 1; position <= (int)kart_amount ; position++)
    {
        Kart *kart = world->getKartAtPosition(position);
        
        if (kart->getPosition() == -1)//if position is not set
        {
            //we use karts ordered by id only
            //(needed for beginning of MINOR_MODE_3_STRIKES)
            kart= world->getKart(position-1);
        }
        
        if(kart->isEliminated()) continue;
        unsigned int kart_id = kart->getWorldKartId();
        
        //x,y is the target position
        int lap = info[kart->getWorldKartId()].lap;
        
        // In battle mode there is no distance along track etc.
        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES)
        {
            x = x_base;
            y = previous_y+ICON_PLAYER_WIDTH+2;
        }
        else
        {
            LinearWorld *linear_world      = (LinearWorld*)(World::getWorld());
            
            float distance = linear_world->getDistanceDownTrackForKart(kart_id)
            + linear_world->getTrack()->getTrackLength()*lap;
            if ((position>1) && (previous_distance-distance<m_dist_show_overlap) && (!kart->hasFinishedRace()))
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
            pos_top.LowerRightCorner   = pos_top.UpperLeftCorner;
            
            font->draw(StringUtils::insertValues( m_string_top, position-1 ), pos_top, color);
            
            break;
        }
        
        if (info[kart_id].m_text.size() > 0)
        {
            video::SColor color = video::SColor(255,
                                                (int)(255*info[kart_id].r),
                                                (int)(255*info[kart_id].g), 
                                                (int)(255*info[kart_id].b)   );
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, 
                                x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s=info[kart_id].m_text.c_str();
            
            font->draw(s.c_str(), pos, color, false, false, NULL, true /* ignore RTL */);
        }
        
        if (info[kart_id].special_title.size() > 0)
        {
            static video::SColor color = video::SColor(255, 255, 0, 0);
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, 
                                x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s(info[kart_id].special_title.c_str());
            font->draw(s.c_str(), pos, color, false, false, NULL, true /* ignore RTL */);
        }
        
        // draw icon
        video::ITexture *icon = 
        kart->getKartProperties()->getIconMaterial()->getTexture();
        int w =
        kart->getController()->isPlayerController() ? ICON_PLAYER_WIDTH
        : ICON_WIDTH;
        const core::rect<s32> pos(x, y, x+w, y+w);
        
        //to bring to light the player's icon: add a background
        if (kart->getController()->isPlayerController())
        {
            video::SColor colors[4];
            for (unsigned int i=0;i<4;i++)
            {
                colors[i]=kart->getKartProperties()->getColor();
                colors[i].setAlpha(
                                   100+(int)(100*cos(M_PI/2*i+World::getWorld()->getTime()*2)));
            }
            const core::rect<s32> rect(core::position2d<s32>(0,0),
                                       m_icons_frame->getTexture()->getOriginalSize());
            irr_driver->getVideoDriver()->draw2DImage(
                                                      m_icons_frame->getTexture(), pos, rect,NULL, colors, true);
        }
        
        // Fixes crash bug, why are certain icons not showing up?
        if ((icon != NULL) && (!kart->playingEmergencyAnimation()) && 
            (!kart->isSquashed())                                     )
        {
            const core::rect<s32> rect(core::position2d<s32>(0,0),
                                       icon->getOriginalSize());
            irr_driver->getVideoDriver()->draw2DImage(icon, pos, rect, 
                                                      NULL, NULL, true);
        }
        
        //draw status info
        
        if ((icon != NULL) && (kart->playingRescueAnimation()))
        {
            //icon fades to the left
            float t_anim=100*sin(0.5f*M_PI*kart->getAnimationTimer());
            const core::rect<s32> rect1(core::position2d<s32>(0,0), 
                                        icon->getOriginalSize());
            const core::rect<s32> pos1((int)(x-t_anim), y, 
                                       (int)(x+w-t_anim), y+w);
            irr_driver->getVideoDriver()->draw2DImage(icon, pos1, rect1, 
                                                      NULL, NULL, true);
        }
        
        if ((icon != NULL) && (!kart->playingEmergencyAnimation()) && (kart->isSquashed()))
        {
            //syncs icon squash with kart squash
            const core::rect<s32> destRect(core::position2d<s32>(x,y+w/4),
                                           core::position2d<s32>(x+w,y+w*3/4));
            const core::rect<s32> sourceRect(core::position2d<s32>(0,0), 
                                             icon->getOriginalSize());
            irr_driver->getVideoDriver()->draw2DImage(icon, destRect, 
                                                      sourceRect, NULL, NULL, 
                                                      true);
        }
        
        if ((icon != NULL) && (kart->playingExplosionAnimation()))
        {
            //exploses into 4 parts
            float t_anim=50.0f*sin(0.5f*M_PI*kart->getAnimationTimer());
            u16 icon_size_x=icon->getOriginalSize().Width;
            u16 icon_size_y=icon->getOriginalSize().Height;
            
            const core::rect<s32> rect1(0, 0, icon_size_x/2,icon_size_y/2);
            const core::rect<s32> pos1((int)(x-t_anim), (int)(y-t_anim), 
                                       (int)(x+w/2-t_anim),
                                       (int)(y+w/2-t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos1, rect1, 
                                                      NULL, NULL, true);
            
            const core::rect<s32> rect2(icon_size_x/2,0,
                                        icon_size_x,icon_size_y/2);
            const core::rect<s32> pos2((int)(x+w/2+t_anim),
                                       (int)(y-t_anim), 
                                       (int)(x+w+t_anim),
                                       (int)(y+w/2-t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos2, rect2,
                                                      NULL, NULL, true);
            
            const core::rect<s32> rect3(0, icon_size_y/2, icon_size_x/2,icon_size_y);
            const core::rect<s32> pos3((int)(x-t_anim), (int)(y+w/2+t_anim), 
                                       (int)(x+w/2-t_anim), (int)(y+w+t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos3, rect3, NULL, NULL, true);
            
            const core::rect<s32> rect4(icon_size_x/2,icon_size_y/2,icon_size_x,icon_size_y);
            const core::rect<s32> pos4((int)(x+w/2+t_anim), (int)(y+w/2+t_anim), 
                                       (int)(x+w+t_anim), (int)(y+w+t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos4, rect4, NULL, NULL, true);
        }
        
        //Plunger
        if (kart->hasViewBlockedByPlunger())
        {
            video::ITexture *icon_plunger = 
            powerup_manager->getIcon(PowerupManager::POWERUP_PLUNGER)->getTexture();
            if (icon_plunger != NULL)
            {
                const core::rect<s32> rect(core::position2d<s32>(0,0), 
                                           icon_plunger->getOriginalSize());
                const core::rect<s32> pos1(x+10, y-10, x+w+10, y+w-10);
                irr_driver->getVideoDriver()->draw2DImage(icon_plunger, pos1, 
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
                                           icon_attachment->getOriginalSize());
                const core::rect<s32> pos1(x-20, y-10, x+w-20, y+w-10);
                irr_driver->getVideoDriver()->draw2DImage(icon_attachment, 
                                                          pos1, rect, NULL, 
                                                          NULL, true);
            }
        }
        
    } //next position
}   // drawGlobalPlayerIcons
