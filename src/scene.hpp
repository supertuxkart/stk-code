//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_SCENE_H
#define HEADER_SCENE_H

//FIXME: make the camera a pointer to vector so it can be forward declared.
#include <vector>
#include "LinearMath/btVector3.h"

class ssgRoot;
class ssgEntity;
class Camera;

class Scene
{
    ssgRoot *m_scenegraph;
    typedef std::vector<Camera*> Cameras;
    Cameras m_cameras;

public:
    void clear();

    Scene  ();
    ~Scene ();
    void reset();

    void add(ssgEntity *kid);
    void remove(ssgEntity *kid);
    void draw(float dt);

    Camera *createCamera(int playerId, const Kart* kart);
};

extern Scene *scene;

#endif
