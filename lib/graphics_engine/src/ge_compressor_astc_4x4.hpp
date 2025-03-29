#ifndef HEADER_GE_ASTC_COMPRESSOR_HPP
#define HEADER_GE_ASTC_COMPRESSOR_HPP

#include "ge_mipmap_generator.hpp"

namespace GE
{
class GECompressorASTC4x4 : public GEMipmap
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
    GECompressorASTC4x4(GEMipmap *mipmap);
    // ------------------------------------------------------------------------
    ~GECompressorASTC4x4()                     { delete [] m_compressed_data; }
};   // GEASTCCompressor

}

#endif
