//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013  Joerg Henrichs
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

#include "graphics/slip_stream.hpp"

#include "config/user_config.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/controller/controller.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "modes/world.hpp"
#include "tracks/quad.hpp"
#include "utils/constants.hpp"

#include <SMesh.h>
#include <SMeshBuffer.h>
#include <IMeshSceneNode.h>

/** Creates the slip stream object using a moving texture.
 *  \param kart Pointer to the kart to which the slip stream
 *              belongs to.
 */
SlipStream::SlipStream(AbstractKart* kart) : MovingTexture(0, 0), m_kart(kart)
{
    video::SMaterial m;
    m.BackfaceCulling = false;
    m.MaterialType    = video::EMT_SOLID;

    Material *material = material_manager->getMaterial("slipstream.png");
    m.setTexture(0, material->getTexture());
    m.setFlag(video::EMF_BACK_FACE_CULLING, false);
    m.setFlag(video::EMF_COLOR_MATERIAL, true);

    m.ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;

    m.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;

    createMesh(m);
    m_node = irr_driver->addMesh(m_mesh, "splistream");
    m_mesh->drop();

#ifdef DEBUG
    std::string debug_name = m_kart->getIdent()+" (slip-stream)";
    m_node->setName(debug_name.c_str());
#endif
    m_node->setPosition(core::vector3df(0,
                                        0*0.25f+2.5,
                                        m_kart->getKartLength()) );
    m_node->setVisible(false);
    setTextureMatrix(&(m_node->getMaterial(0).getTextureMatrix(0)));
    m_slipstream_time      = 0.0f;

    float length = m_kart->getKartProperties()->getSlipstreamLength();
    float kw     = m_kart->getKartWidth();
    float ew     = m_kart->getKartProperties()->getSlipstreamWidth();
    float kl     = m_kart->getKartLength();

    Vec3 p[4];
    p[0]=Vec3(-kw*0.5f, 0, -kl*0.5f       );
    p[1]=Vec3(-ew*0.5f, 0, -kl*0.5f-length);
    p[2]=Vec3( ew*0.5f, 0, -kl*0.5f-length);
    p[3]=Vec3( kw*0.5f, 0, -kl*0.5f       );
    m_slipstream_original_quad = new Quad(p[0], p[1], p[2], p[3]);
    m_slipstream_quad          = new Quad(p[0], p[1], p[2], p[3]);
    if(UserConfigParams::m_slipstream_debug)
    {
        video::SMaterial material;
        material.MaterialType    = video::EMT_TRANSPARENT_ADD_COLOR;
        material.setFlag(video::EMF_BACK_FACE_CULLING, false);
        material.setFlag(video::EMF_LIGHTING, false);

        m_debug_mesh = irr_driver->createQuadMesh(&material, true);
        scene::IMeshBuffer *buffer = m_debug_mesh->getMeshBuffer(0);
        assert(buffer->getVertexType()==video::EVT_STANDARD);
        irr::video::S3DVertex* vertices
            = (video::S3DVertex*)buffer->getVertices();
        video::SColor red(128, 255, 0, 0);
        for(unsigned int i=0; i<4; i++)
        {
            vertices[i].Pos   = p[i].toIrrVector();
            vertices[i].Color = red;
        }
        buffer->recalculateBoundingBox();
        m_mesh->setBoundingBox(buffer->getBoundingBox());
        m_debug_node = irr_driver->addMesh(m_debug_mesh, "splistream_debug", m_kart->getNode());
        m_debug_node->grab();
    }
    else
    {
        m_debug_mesh = NULL;
        m_debug_node = NULL;
    }

}   // SlipStream

//-----------------------------------------------------------------------------
/** Removes the node from the scene graph.
 */
SlipStream::~SlipStream()
{
    irr_driver->removeNode(m_node);
    if(m_debug_node)
    {
        m_debug_node->drop();
        m_debug_mesh->drop();
    }
    delete m_slipstream_original_quad;
    delete m_slipstream_quad;

}   // ~SlipStream

//-----------------------------------------------------------------------------
/** Called at re-start of a race. */
void SlipStream::reset()
{
    m_slipstream_mode = SS_NONE;
    m_slipstream_time = 0;

    // Reset a potential max speed increase
    m_kart->increaseMaxSpeed(MaxSpeed::MS_INCREASE_SLIPSTREAM, 0, 0, 0, 0);
}   // reset

//-----------------------------------------------------------------------------
/** Creates the mesh for the slipstream effect. This function creates a
 *  first a series of circles (with a certain number of vertices each and
 *  distance from each other. Then it will create the triangles and add
 *  texture coordniates.
 *  \param material  The material to use.
 */
void SlipStream::createMesh(const video::SMaterial &material)
{
    // All radius, starting with the one closest to the kart (and
    // widest) to the one furthest away. A 0 indicates the end of the list
    float radius[] = {1.5f, 1.0f, 0.5f, 0.0f};

    // The distance of each of the circle from the kart. The number of
    // entries in this array must be the same as the number of non-zero
    // entries in the radius[] array above. No 'end of list' entry required.
    // Note also that in order to avoid a 'bent' in the texture the
    // difference between the distances must be linearly correlated to the
    // difference in the corresponding radius, see the following sideview:
    //            +
    //       +    |
    //  +    |    |    three radius
    //  |    |    |
    //  +----+----+
    //  0    1    2
    //  distances
    // (radius1-radius0)/(distance1-distance0) = (radius2-radius1)/(distnace2-distance0)
    // This way the line connecting the upper "+" is a straight line,
    // and so the 3d cone shape will not be disturbed.
    float distance[] = {2.0f, 6.0f, 10.0f };

    // The alpha values for the rings, no 'end of list' entry required.
    int alphas[]     = {0, 255, 0};

    // Loop through all given radius to determine the number
    // of segments to create.
    unsigned int num_circles=0;
    while(radius[num_circles]>0.0f) num_circles++;

    assert(num_circles > 0);

    // Length is distance of last circle to distance of first circle:
    m_length = distance[num_circles-1] - distance[0];

    // The number of points for each circle. Since part of the slip stream
    // might be under the ground (esp. first and last segment), specify
    // which one is the first and last to be actually drawn.
    const unsigned int  num_segments   = 15;
    const unsigned int  first_segment  = 0;
    const unsigned int  last_segment   = 14;
    const float         f              = 2*M_PI/float(num_segments);
    scene::SMeshBuffer *buffer         = new scene::SMeshBuffer();
    buffer->Material                   = material;
    for(unsigned int j=0; j<num_circles; j++)
    {
        float curr_distance = distance[j]-distance[0];
        // Create the vertices for each of the circle
        for(unsigned int i=first_segment; i<=last_segment; i++)
        {
            video::S3DVertex v;
            // Offset every 2nd circle by one half segment to increase
            // the number of planes so it looks better.
            v.Pos.X =  sin((i+(j%2)*0.5f)*f)*radius[j];
            v.Pos.Y = -cos((i+(j%2)*0.5f)*f)*radius[j];
            v.Pos.Z = distance[j];
            v.Color = video::SColor(alphas[j], alphas[j], alphas[j], alphas[j]);
            v.TCoords.X = curr_distance/m_length;
            v.TCoords.Y = (float)(i-first_segment)/(last_segment-first_segment)
                          + (j%2)*(.5f/num_segments);
            buffer->Vertices.push_back(v);
        }   // for i<num_segments
    }   // while radius[num_circles]!=0

    // Now create the triangles from circle j to j+1 (so the loop
    // only goes to num_circles-1).
    const int diff_segments = last_segment-first_segment+1;
    for(unsigned int j=0; j<num_circles-1; j++)
    {
        for(unsigned int i=first_segment; i<last_segment; i++)
        {
            buffer->Indices.push_back( j   *diff_segments+i  );
            buffer->Indices.push_back((j+1)*diff_segments+i  );
            buffer->Indices.push_back( j   *diff_segments+i+1);
            buffer->Indices.push_back( j   *diff_segments+i+1);
            buffer->Indices.push_back((j+1)*diff_segments+i  );
            buffer->Indices.push_back((j+1)*diff_segments+i+1);
        }
    }   // for j<num_circles-1

    scene::SMesh *mesh = new scene::SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();

    buffer->drop();
    m_mesh = mesh;
}   // createMesh

//-----------------------------------------------------------------------------
/** Sets the animation intensity (or speed).
 *  \param f Intensity: 0 = no slip stream,
 *                      1 = collecting
 *                      2 = using slip stream bonus
 */
void SlipStream::setIntensity(float f, const AbstractKart *kart)
{
    if(!kart)
    {
        m_node->setVisible(false);
        return;
    }

    m_node->setVisible(true);
    const float above_terrain = 0.2f;
    core::vector3df my_pos = m_kart->getNode()->getPosition();
    my_pos.Y = m_kart->getHoT()+above_terrain;
    m_node->setPosition(my_pos);

    core::vector3df other_pos = kart->getNode()->getPosition();
    other_pos.Y = kart->getHoT()+above_terrain;
    core::vector3df diff =   other_pos - my_pos;
    core::vector3df rotation = diff.getHorizontalAngle();
    m_node->setRotation(rotation);
    float fs = diff.getLength()/m_length;
    m_node->setScale(core::vector3df(1, 1, fs));

    // For real testing in game: this needs some tuning!
    m_node->setVisible(f!=0);
    MovingTexture::setSpeed(f, 0);


    int c = (int)(f*255);
    if (c > 255) c = 255;

    const unsigned int bcount = m_node->getMesh()->getMeshBufferCount();
    for (unsigned int b=0; b<bcount; b++)
    {
        scene::IMeshBuffer* mb = m_node->getMesh()->getMeshBuffer(b);
        irr::video::S3DVertex* vertices = (video::S3DVertex*)mb->getVertices();
        for (unsigned int i=0; i<mb->getVertexCount(); i++)
        {
            const int color = (int)(c*(vertices[i].Color.getAlpha()/255.0f));
            vertices[i].Color.setRed( color );
            vertices[i].Color.setGreen( color );
            vertices[i].Color.setBlue( color );
        }
    }

    return;
    // For debugging: make the slip stream effect visible all the time
    m_node->setVisible(true);
    MovingTexture::setSpeed(1.0f, 0.0f);
}   // setIntensity

//-----------------------------------------------------------------------------
/** Returns true if enough slipstream credits have been accumulated
*  to get a boost when leaving the slipstream area.
*/
bool SlipStream::isSlipstreamReady() const
{
    return m_slipstream_time>
        m_kart->getKartProperties()->getSlipstreamCollectTime();
}   // isSlipstreamReady

//-----------------------------------------------------------------------------
/** Returns the additional force being applied to the kart because of
 *  slipstreaming.
 */
void SlipStream::updateSlipstreamPower()
{
    // See if we are currently using accumulated slipstream credits:
    // -------------------------------------------------------------
    if(m_slipstream_mode==SS_USE)
    {
        setIntensity(2.0f, NULL);
        const KartProperties *kp=m_kart->getKartProperties();
        m_kart->increaseMaxSpeed(MaxSpeed::MS_INCREASE_SLIPSTREAM,
                                kp->getSlipstreamMaxSpeedIncrease(),
                                kp->getSlipstreamAddPower(),
                                kp->getSlipstreamDuration(),
                                kp->getSlipstreamFadeOutTime()       );
    }
}   // upateSlipstreamPower

//-----------------------------------------------------------------------------
/** Sets the color of the debug mesh (which shows the area in which slipstream
 *  can be accumulated).
 *  Color codes:
 *  black:  kart too slow
 *  red:    not inside of slipstream area
 *  green:  slipstream is being accumulated.
 */
void SlipStream::setDebugColor(const video::SColor &color)
{
    if(!UserConfigParams::m_slipstream_debug) return;
    scene::IMeshBuffer *buffer = m_debug_mesh->getMeshBuffer(0);
    irr::video::S3DVertex* vertices =
        (video::S3DVertex*)buffer->getVertices();
    for(unsigned int i=0; i<4; i++)
        vertices[i].Color=color;
}   // setDebugColor

//-----------------------------------------------------------------------------
/** Update, called once per timestep.
 *  \param dt Time step size.
 */
void SlipStream::update(float dt)
{
    // Low level AIs should not do any slipstreaming.
    if(m_kart->getController()->disableSlipstreamBonus())
        return;

    MovingTexture::update(dt);

    // Update this karts slipstream quad (even for low level AI which don't
    // use slipstream, since even then player karts can get slipstream,
    // and so have to compare with the modified slipstream quad.
    m_slipstream_original_quad->transform(m_kart->getTrans(),
                                          m_slipstream_quad);

    if(m_slipstream_mode==SS_USE)
    {
        m_slipstream_time -= dt;
        if(m_slipstream_time<0) m_slipstream_mode=SS_NONE;
    }

    updateSlipstreamPower();

    // If this kart is too slow for slipstreaming taking effect, do nothing
    // --------------------------------------------------------------------
    // Define this to get slipstream effect shown even when the karts are
    // not moving. This is useful for debugging the graphics of SS-ing.
#undef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
#ifndef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
    if(m_kart->getSpeed()<m_kart->getKartProperties()->getSlipstreamMinSpeed())
    {
        setIntensity(0, NULL);
        m_slipstream_mode = SS_NONE;
        if(UserConfigParams::m_slipstream_debug)
            setDebugColor(video::SColor(255, 0, 0, 0));
        return;
    }
#endif

    // Then test if this kart is in the slipstream range of another kart:
    // ------------------------------------------------------------------
    World *world           = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    bool is_sstreaming     = false;
    m_target_kart          = NULL;

    // Note that this loop can not be simply replaced with a shorter loop
    // using only the karts with a better position - since a kart might
    // be a lap behind
    for(unsigned int i=0; i<num_karts; i++)
    {
        m_target_kart= world->getKart(i);
        // Don't test for slipstream with itself, a kart that is being
        // rescued or exploding, or an eliminated kart
        if(m_target_kart==m_kart               ||
            m_target_kart->getKartAnimation()  ||
            m_target_kart->isEliminated()        ) continue;

        float diff = fabsf(m_target_kart->getXYZ().getY()
                           - m_kart->getXYZ().getY()      );
        // If the kart is 'on top' of this kart (e.g. up on a bridge),
        // don't consider it for slipstreaming.

        if(diff>6.0f) continue;
        // If the kart we are testing against is too slow, no need to test
        // slipstreaming. Note: We compare the speed of the other kart
        // against the minimum slipstream speed kart of this kart - not
        // entirely sure if this makes sense, but it makes it easier to
        // give karts different slipstream properties.
#ifndef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
        if(m_target_kart->getSpeed() <
            m_kart->getKartProperties()->getSlipstreamMinSpeed())
        {
            if(UserConfigParams::m_slipstream_debug &&
                m_kart->getController()->isPlayerController())
                m_target_kart->getSlipstream()
                              ->setDebugColor(video::SColor(255, 0, 0, 0));

            continue;
        }
#endif
        // Quick test: the kart must be not more than
        // slipstream length+0.5*kart_length()+0.5*target_kart_length
        // away from the other kart
        Vec3 delta = m_kart->getXYZ() - m_target_kart->getXYZ();
        float l    = m_target_kart->getKartProperties()->getSlipstreamLength()
                   + 0.5f*( m_target_kart->getKartLength()
                           +m_kart->getKartLength()        );
        if(delta.length2_2d() > l*l)
        {
            if(UserConfigParams::m_slipstream_debug &&
                m_kart->getController()->isPlayerController())
                m_target_kart->getSlipstream()
                             ->setDebugColor(video::SColor(255, 0, 0, 128));
            continue;
        }
        // Real test: if in slipstream quad of other kart
        if(m_target_kart->getSlipstream()->m_slipstream_quad
                                         ->pointInQuad(m_kart->getXYZ()))
        {
            is_sstreaming     = true;
            break;
        }
        if(UserConfigParams::m_slipstream_debug &&
            m_kart->getController()->isPlayerController())
            m_target_kart->getSlipstream()
                         ->setDebugColor(video::SColor(255, 0, 0, 255));
    }   // for i < num_karts

    if(!is_sstreaming)
    {
        if(UserConfigParams::m_slipstream_debug &&
            m_kart->getController()->isPlayerController())
            m_target_kart->getSlipstream()
                         ->setDebugColor(video::SColor(255, 255, 0, 0));

        if(isSlipstreamReady())
        {
            // The first time slipstream is ready after collecting
            // and you are leaving the slipstream area, you get a
            // zipper bonus.
            if(m_slipstream_mode==SS_COLLECT)
            {
                m_slipstream_mode = SS_USE;
                m_kart->handleZipper();
                m_slipstream_time =
                    m_kart->getKartProperties()->getSlipstreamCollectTime();
                return;
            }
        }
        m_slipstream_time -=dt;
        if(m_slipstream_time<0) m_slipstream_mode = SS_NONE;
        setIntensity(0, NULL);
        return;
    }   // if !is_sstreaming

    if(UserConfigParams::m_slipstream_debug &&
        m_kart->getController()->isPlayerController())
        m_target_kart->getSlipstream()->setDebugColor(video::SColor(255, 0, 255, 0));
    // Accumulate slipstream credits now
    m_slipstream_time = m_slipstream_mode==SS_NONE ? dt
                                                   : m_slipstream_time+dt;
    if(isSlipstreamReady())
        m_kart->setSlipstreamEffect(9.0f);
    setIntensity(m_slipstream_time, m_target_kart);

    m_slipstream_mode = SS_COLLECT;
    if(m_slipstream_time>m_kart->getKartProperties()->getSlipstreamCollectTime())
    {
        setIntensity(1.0f, m_target_kart);
    }
}   // update
