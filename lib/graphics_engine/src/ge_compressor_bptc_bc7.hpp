#ifndef HEADER_GE_COMPRESSOR_BPTC_BC7_HPP
#define HEADER_GE_COMPRESSOR_BPTC_BC7_HPP

#include "ge_mipmap_generator.hpp"

namespace GE
{
class GECompressorBPTCBC7 : public GEMipmapGenerator
{
private:
    uint8_t* m_compressed_data;
public:
    // ------------------------------------------------------------------------
    static void init();
    // ------------------------------------------------------------------------
    GECompressorBPTCBC7(uint8_t* texture, unsigned channels,
                        const irr::core::dimension2d<irr::u32>& size,
                        bool normal_map);
    // ------------------------------------------------------------------------
    ~GECompressorBPTCBC7()                     { delete [] m_compressed_data; }
};   // GECompressorBPTCBC7

}

#endif
