#ifndef HEADER_GE_ASTC_COMPRESSOR_HPP
#define HEADER_GE_ASTC_COMPRESSOR_HPP

#include "ge_mipmap_generator.hpp"

namespace GE
{
class GECompressorASTC4x4 : public GEMipmapGenerator
{
private:
    uint8_t* m_compressed_data;
public:
    // ------------------------------------------------------------------------
    static void init();
    // ------------------------------------------------------------------------
    static void destroy();
    // ------------------------------------------------------------------------
    static bool loaded();
    // ------------------------------------------------------------------------
    GECompressorASTC4x4(uint8_t* texture, unsigned channels,
                        const irr::core::dimension2d<irr::u32>& size,
                        bool normal_map);
    // ------------------------------------------------------------------------
    ~GECompressorASTC4x4()                     { delete [] m_compressed_data; }
};   // GEASTCCompressor

}

#endif
