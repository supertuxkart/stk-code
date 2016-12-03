//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#include "graphics/materials.hpp"

const STK::Tuple<size_t> DefaultMaterial::FirstPassTextures
    = STK::Tuple<size_t>(1);
const STK::Tuple<size_t, size_t, size_t> DefaultMaterial::SecondPassTextures
    = STK::Tuple<size_t, size_t, size_t>(0, 1, 2);
const STK::Tuple<> DefaultMaterial::ShadowTextures;
const STK::Tuple<size_t> DefaultMaterial::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t, size_t> AlphaRef::FirstPassTextures
    = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t, size_t> AlphaRef::SecondPassTextures
    = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t> AlphaRef::ShadowTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> AlphaRef::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t> SphereMap::FirstPassTextures = STK::Tuple<size_t>(1);
const STK::Tuple<size_t> SphereMap::SecondPassTextures = STK::Tuple<size_t>(0);
const STK::Tuple<> SphereMap::ShadowTextures;
const STK::Tuple<size_t> SphereMap::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t, size_t> UnlitMat::FirstPassTextures
    = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t> UnlitMat::SecondPassTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> UnlitMat::ShadowTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> UnlitMat::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t, size_t> GrassMat::FirstPassTextures
    = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t, size_t, size_t> GrassMat::SecondPassTextures
    = STK::Tuple<size_t, size_t, size_t>(0, 1, 2);
const STK::Tuple<size_t> GrassMat::ShadowTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> GrassMat::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t, size_t> NormalMat::FirstPassTextures
    = STK::Tuple<size_t, size_t>(3, 1);
const STK::Tuple<size_t, size_t, size_t> NormalMat::SecondPassTextures
    = STK::Tuple<size_t, size_t, size_t>(0, 1, 2);
const STK::Tuple<> NormalMat::ShadowTextures;
const STK::Tuple<size_t> NormalMat::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t> DetailMat::FirstPassTextures = STK::Tuple<size_t>(1);
const STK::Tuple<size_t, size_t, size_t> DetailMat::SecondPassTextures
    = STK::Tuple<size_t, size_t, size_t>(0, 3, 1);
const STK::Tuple<> DetailMat::ShadowTextures;
const STK::Tuple<size_t> DetailMat::RSMTextures = STK::Tuple<size_t>(0);

// ----------------------------------------------------------------------------
const STK::Tuple<size_t> SplattingMat::FirstPassTextures = STK::Tuple<size_t>(7);
const STK::Tuple<size_t, size_t, size_t, size_t, size_t>
          SplattingMat::SecondPassTextures
              = STK::Tuple<size_t, size_t, size_t, size_t, size_t>(1, 3, 4, 5, 6);
const STK::Tuple<> SplattingMat::ShadowTextures;
const STK::Tuple<size_t, size_t, size_t, size_t, size_t> SplattingMat::RSMTextures
    = STK::Tuple<size_t, size_t, size_t, size_t, size_t>(1, 3, 4, 5, 6);
