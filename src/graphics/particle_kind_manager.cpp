//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs, Marianne Gagnon
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

#include "graphics/particle_kind_manager.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include <stdexcept>

// ----------------------------------------------------------------------------

ParticleKindManager* ParticleKindManager::singleton = NULL;

ParticleKindManager* ParticleKindManager::get()
{
    if (singleton == NULL) singleton = new ParticleKindManager();
    return singleton;
}

// ----------------------------------------------------------------------------

ParticleKindManager::ParticleKindManager()
{
}

// ----------------------------------------------------------------------------

ParticleKindManager::~ParticleKindManager()
{
    cleanup();
}

// ----------------------------------------------------------------------------

void ParticleKindManager::cleanup()
{
    cleanUpTrackSpecificGfx();

    std::map<std::string, ParticleKind*>::iterator it;
    for (it = m_kinds.begin(); it != m_kinds.end(); it++)
    {
        delete it->second;
    }
    m_kinds.clear();
}

// ----------------------------------------------------------------------------

void ParticleKindManager::cleanUpTrackSpecificGfx()
{
    std::map<std::string, ParticleKind*>::iterator it;
    for (it = m_per_track_kinds.begin(); it != m_per_track_kinds.end(); it++)
    {
        delete it->second;
    }
    m_per_track_kinds.clear();
}

// ----------------------------------------------------------------------------

ParticleKind* ParticleKindManager::getParticles(const std::string &name)
{
    std::map<std::string, ParticleKind*>::iterator i;
    i = m_per_track_kinds.find(name);
    if (i != m_per_track_kinds.end())
    {
        return i->second;
    }
    else
    {
        try
        {
            Track* t = track_manager->getTrack(race_manager->getTrackName());
            if (t)
            {
                ParticleKind* newkind = new ParticleKind(t->getTrackFile(name));
                m_per_track_kinds[name] = newkind;
                return newkind;
            }
        }
        catch (std::runtime_error& e)
        {
            (void)e;   // remove compiler warning
            // not found in track directory, let's try globally...
        }
    }

    i = m_kinds.find(name);
    if (i == m_kinds.end())
    {
        try
        {
            std::string path = file_manager->getAsset(FileManager::GFX,name);
            ParticleKind* newkind = new ParticleKind(path);
            m_kinds[name] = newkind;
            return newkind;
        }
        catch (std::runtime_error& e)
        {
            (void)e;  // avoid compiler warning
            return NULL;
        }
    }
    else
    {
        return i->second;
    }
}

// ----------------------------------------------------------------------------
bool ParticleKindManager::isGlobalParticleMaterial(Material* m) const
{
    for (auto& p : m_kinds)
    {
        if (p.second->getMaterial() == m)
        {
            return true;
        }
    }
    return false;
}
