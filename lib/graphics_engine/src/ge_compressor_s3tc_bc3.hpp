#ifndef HEADER_GE_COMPRESSOR_S3TC_BC3_HPP
#define HEADER_GE_COMPRESSOR_S3TC_BC3_HPP

#include "ge_mipmap_generator.hpp"

namespace GE
{
class GECompressorS3TCBC3 : public GEMipmapGenerator
{
private:
    uint8_t* m_compressed_data;
public:
    // ------------------------------------------------------------------------
    GECompressorS3TCBC3(uint8_t* texture, unsigned channels,
                        const irr::core::dimension2d<irr::u32>& size,
                        bool normal_map);
    // ------------------------------------------------------------------------
    ~GECompressorS3TCBC3()                     { delete [] m_compressed_data; }
};   // GECompressorS3TCBC3

}

#endif
