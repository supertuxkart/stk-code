#ifndef HEADER_GE_COMPRESSOR_BPTC_BC7_HPP
#define HEADER_GE_COMPRESSOR_BPTC_BC7_HPP

#include "ge_mipmap_generator.hpp"

namespace GE
{
class GECompressorBPTCBC7 : public GEMipmap
{
private:
    uint8_t* m_compressed_data;
public:
    // ------------------------------------------------------------------------
    static void init();
    // ------------------------------------------------------------------------
    GECompressorBPTCBC7(GEMipmap *mipmap);
    // ------------------------------------------------------------------------
    ~GECompressorBPTCBC7()                     { delete [] m_compressed_data; }
};   // GECompressorBPTCBC7

}

#endif
