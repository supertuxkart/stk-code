#ifndef HEADER_GE_ENVIRONMENT_MAP_HPP
#define HEADER_GE_ENVIRONMENT_MAP_HPP

#include "ge_mipmap_generator.hpp"

#include "quaternion.h"
#include "SColor.h"
#include "vector3d.h"

#include <array>

namespace GE
{

class GECubemapSampler
{
private:
    irr::video::SColorf *m_data;

    std::array<std::vector<GEImageLevel>, 6> m_map;

    unsigned m_max_size;

public:
    // ------------------------------------------------------------------------
    GECubemapSampler(std::array<GEMipmap*, 6> cubemap, unsigned max_size = 256u);
    // ------------------------------------------------------------------------
    ~GECubemapSampler()                                   { delete [] m_data; }
    // ------------------------------------------------------------------------
    unsigned getMaxSize() const                          { return m_max_size; }
    // ------------------------------------------------------------------------
    irr::video::SColorf sample(irr::core::vector3df dir, float lod) const;
};

class GEEnvironmentMap : public GEMipmap
{
private:

    uint8_t* m_data;

public:

    // ------------------------------------------------------------------------
    static const unsigned getIrradianceResolution()             { return 16u; }
    // ------------------------------------------------------------------------
    static const unsigned getIrradianceNumSamples()            { return 256u; }
    // ------------------------------------------------------------------------
    static const unsigned getRadianceResolution()              { return 256u; }
    // ------------------------------------------------------------------------
    static const unsigned getRadianceNumSamples()               { return 32u; }
    // ------------------------------------------------------------------------
    GEEnvironmentMap(GEMipmap *mipmap, GECubemapSampler &sampler,
                     int face, bool diffuse,
                     irr::core::quaternion occlusion_sh2 = 
                     irr::core::quaternion(0.f, 2.f, 0.f, 0.f));
    // ------------------------------------------------------------------------
    ~GEEnvironmentMap()                                   { delete [] m_data; }
};   // GECullingTool

}

#endif
