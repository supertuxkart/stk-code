#include "modes/tutorial_world.hpp"

#include "karts/kart.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"

TutorialWorld::TutorialWorld()
{
    m_stop_music_when_dialog_open = false;
}

// -----------------------------------------------------------------------------

void TutorialWorld::moveKartAfterRescue(AbstractKart* kart)
{
    float angle = 0;
    
    // find closest point to drop kart on
    World *world = World::getWorld();
    const int start_spots_amount = 
        world->getTrack()->getNumberOfStartPositions();
    assert(start_spots_amount > 0);
    
    const float currentKart_x = kart->getXYZ().getX();
    const float currentKart_z = kart->getXYZ().getZ();

    const btTransform& s = getClosestStartPoint(currentKart_x, currentKart_z);
    const Vec3 &xyz = s.getOrigin();
    kart->setXYZ(xyz);
    kart->setRotation(s.getRotation());

    //position kart from same height as in World::resetAllKarts
    btTransform pos;
    pos.setOrigin( kart->getXYZ()
                  +btVector3(0, 0.5f*kart->getKartHeight(), 0.0f));
    pos.setRotation( btQuaternion(btVector3(0.0f, 1.0f, 0.0f), angle) );

    kart->getBody()->setCenterOfMassTransform(pos);

    //project kart to surface of track
    bool kart_over_ground = m_physics->projectKartDownwards(kart);

    if (kart_over_ground)
    {
        //add vertical offset so that the kart starts off above the track
        float vertical_offset = 
              kart->getKartProperties()->getVertRescueOffset()
            * kart->getKartHeight();
        kart->getBody()->translate(btVector3(0, vertical_offset, 0));
    }
    else
    {
        fprintf(stderr, "WARNING: invalid position after rescue for kart %s"
                         "on track %s.\n",
                (kart->getIdent().c_str()), m_track->getIdent().c_str());
    }

}   // moveKartAfterRescue

// -----------------------------------------------------------------------------

btTransform TutorialWorld::getClosestStartPoint(float currentKart_x,
                                                float currentKart_z)
{
    // find closest point to drop kart on
    World *world = World::getWorld();
    const int start_spots_amount = 
        world->getTrack()->getNumberOfStartPositions();
    assert(start_spots_amount > 0);
    
    
    int closest_id = -1;
    float closest_distance = 999999999.0f;

    for (int n=0; n<start_spots_amount; n++)
    {
        // no need for the overhead to compute exact distance with sqrt(), 
        // so using the 'manhattan' heuristic which will do fine enough.
        const btTransform &s = world->getTrack()->getStartTransform(n);
        const Vec3 &v = s.getOrigin();
                
        float absDistance = fabs(currentKart_x - v.getX()) +
                    fabs(currentKart_z - v.getZ());
        
        if (absDistance < closest_distance)
        {
            closest_distance = absDistance;
            closest_id = n;
        }
    }
    
    assert(closest_id != -1);
    return world->getTrack()->getStartTransform(closest_id);
}   // getClosestStartPoint
